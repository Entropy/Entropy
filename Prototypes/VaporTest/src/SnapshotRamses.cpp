#include "SnapshotRamses.h"
#include "Constants.h"

#ifdef TARGET_LINUX
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif
namespace ent
{

	void readToMemory(const std::string & path, size_t size, char * buffer){
    #if defined(TARGET_LINUX) && FAST_READ
		auto fd = open (ofToDataPath(path).c_str(), O_RDONLY, S_IRUSR);
		char * data = (char*)mmap (nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0);
		if(data == (const char *)-1){
			ofLogError() << "Couldn't read memory mapped file";
			return;
		}
		memcpy(buffer, data, size);
		munmap(data, size);
		close(fd);
    #else
		ofFile file(path, ofFile::ReadOnly, true);
		file.read(buffer, size);
    #endif
	}

	void readToBuffer(const std::string & path, size_t size, ofBufferObject & buffer){
		auto data = buffer.map<char>(GL_WRITE_ONLY);
		readToMemory(path, size, data);
		buffer.unmap();
	}

	//--------------------------------------------------------------
	SnapshotRamses::SnapshotRamses()
	{
		clear();
	}
	
	//--------------------------------------------------------------
	SnapshotRamses::~SnapshotRamses()
	{
		clear();
	}

	void SnapshotRamses::precalculate(const std::string folder, int frameIndex, float minDensity, float maxDensity, size_t worldsize){
		auto rawFileName = ofFilePath::removeTrailingSlash(folder) + ".raw";
		auto particlesFileName = ofFilePath::removeTrailingSlash(folder) + ".particles";
		auto metaFileName = ofFilePath::removeTrailingSlash(folder) + "_meta.raw";
		auto voxelsFileName = ofFilePath::removeTrailingSlash(folder) + "_voxels.raw";
		auto particlesGroupsFileName = ofFilePath::removeTrailingSlash(folder) + ".groups";

		//------------------------------------
		// Load the HDF5 data.
		std::vector<float> posX;
		std::vector<float> posY;
		std::vector<float> posZ;
		std::vector<float> cellSize;
		std::vector<float> density;

		auto then = ofGetElapsedTimeMicros();
		loadhdf5(folder + "x/seq_" + ofToString(frameIndex) + "_x.h5", posX);
		loadhdf5(folder + "y/seq_" + ofToString(frameIndex) + "_y.h5", posY);
		loadhdf5(folder + "z/seq_" + ofToString(frameIndex) + "_z.h5", posZ);
		loadhdf5(folder + "dx/seq_" + ofToString(frameIndex) + "_dx.h5", cellSize);
		loadhdf5(folder + "density/seq_" + ofToString(frameIndex) + "_density.h5", density);
		auto now = ofGetElapsedTimeMicros();
		cout << "time to load original files " << float(now - then)/1000 << "ms." << endl;

		std::vector<Particle> particles(posX.size());
		for(size_t i=0;i<posX.size();++i){
			particles[i] = {{posX[i], posY[i], posZ[i]}, cellSize[i], density[i]};
		}

		//------------------------------------
		// Precalculate ranges
		{
			m_numCells = posX.size();

			// Set the ranges for all data.
			for (size_t i = 0; i < posX.size(); ++i)
			{
				m_densityRange.add(density[i]);
				m_coordRange.add(glm::vec3(posX[i], posY[i], posZ[i]));
				m_sizeRange.add(cellSize[i]);
			}

			auto threshold = minDensity * m_densityRange.getMin() + (maxDensity * m_densityRange.getMax() - minDensity * m_densityRange.getMin()) * 0.001f;
			ofxRange3f range;
			range.clear();
			range = std::accumulate(particles.begin(), particles.end(), range, [&](ofxRange3f range, const Particle & p){
			    if(p.density>threshold){
			        range.add(p.pos.xyz());
		        }
			    return range;
		    });

			cout << "num particles after filter: " << particles.size() << endl;
			auto min = m_coordRange.getMin();
			auto max = m_coordRange.getMax();
			cout << min.x << ", " << min.y << ", " << min.z << " - " << max.x << ", " << max.y << ", " << max.z << endl;
			cout << "size range " << m_sizeRange.getMin() << " - " << m_sizeRange.getMax() << endl;

			// Expand coord range taking cell size into account.
			m_coordRange.add(m_coordRange.getMin() - m_sizeRange.getMax());
			m_coordRange.add(m_coordRange.getMax() + m_sizeRange.getMax());
			range.add(range.getMin() - m_sizeRange.getMax());
			range.add(range.getMax() + m_sizeRange.getMax());

			// Find the dimension with the max span, and set all spans to be the same (since we're rendering a cube).
			glm::vec3 coordSpan = m_coordRange.getSpan();
			float maxSpan = MAX(coordSpan.x, MAX(coordSpan.y, coordSpan.z));
			glm::vec3 boxCoordSpan = range.getSpan();
			float maxBoxSpan = std::max(boxCoordSpan.x, std::max(boxCoordSpan.y, boxCoordSpan.z));
			float minBoxSpan = std::min(boxCoordSpan.x, std::min(boxCoordSpan.y, boxCoordSpan.z));
			glm::vec3 spanOffset(maxSpan * 0.5);
			glm::vec3 coordMid = m_coordRange.getCenter();
			m_coordRange.add(coordMid - spanOffset);
			m_coordRange.add(coordMid + spanOffset);
			//range.add(range.getCenter() - maxBoxSpan);
			//range.add(range.getCenter() + maxBoxSpan);

			m_boxRange = BoundingBox(m_coordRange.getCenter(), glm::vec3(minBoxSpan));
		}

		//------------------------------------
		// Precalculate 3d texture pixels and
		// order particles so they don't overlap
		// in memory when recreating the texture
		// with the compute shader
		{
			ofxRange3f range;
			range.clear();
			range.add(m_boxRange.center);
			range.add(m_boxRange.min);
			range.add(m_boxRange.max);

			then = ofGetElapsedTimeMicros();
			this->vaporPixels.setup(particles, worldsize, minDensity * m_densityRange.getMin(), maxDensity * m_densityRange.getMax(), range);
			now = ofGetElapsedTimeMicros();
			cout << "time to compute 3D texture " << float(now - then)/1000 << "ms." << endl;
			m_numCells = vaporPixels.getParticlesInBox().size();
			cout << "octree num particles " << m_numCells << endl;
		}

		/*then = ofGetElapsedTimeMicros();
		cout << "calculating octree with " << log2(worldsize*2) << " levels" << endl;
		this->vaporOctree.setup(particles);
		this->vaporOctree.compute(log2(worldsize*2), minDensity * m_densityRange.getMin(), maxDensity * m_densityRange.getMax());
		now = ofGetElapsedTimeMicros();
		cout << "time to compute octree " << float(now - then)/1000 << "ms." << endl;
		auto octree_particles = vaporOctree.toVector();*/



		//------------------------------------
		// Write data to files
		{
			ofFile texture3dRaw(rawFileName, ofFile::WriteOnly, true);
			texture3dRaw.write((const char*)this->vaporPixels.data().data(), this->vaporPixels.data().size() * sizeof(float));

			ofFile texture3dMeta(metaFileName, ofFile::WriteOnly, true);
			texture3dMeta << m_densityRange.getMin() << " " << m_densityRange.getMax() << " " <<
			                m_coordRange.getMin() << " " << m_coordRange.getMax() << " " <<
			                m_sizeRange.getMin() << " " << m_sizeRange.getMax() << " " <<
			                m_boxRange.min << " " << m_boxRange.max << " " <<
			                vaporPixels.getParticlesInBox().size() << " " <<
			                vaporPixels.getGroupIndices().size();


			ofFile particlesFile(particlesFileName, ofFile::WriteOnly, true);
			particlesFile.write((const char*)vaporPixels.getParticlesInBox().data(), vaporPixels.getParticlesInBox().size() * sizeof(Particle));
			ofFile particlesGroupsFile(particlesGroupsFileName, ofFile::WriteOnly, true);
			particlesGroupsFile.write((const char*)vaporPixels.getGroupIndices().data(), sizeof(size_t) * vaporPixels.getGroupIndices().size());
		}



		//------------------------------------
		// Find out voxels that contribute more
		// to the final texture and remove the rest
		// as a form of lossy compression
		{
			std::array<size_t,100> histogram = {{0}};
			std::array<double,100> histogram_contribution = {{0.}};
			ofxRange3f zero;
			zero.clear();
			std::array<ofxRange3f,100> histogram_ranges = {{ zero }};
			std::array<float,100> histogram_factor = {{ 0.f }};
			double totalDensity = 0.;

			for(uint32_t z = 0, i=0; z<worldsize; z++){
				for(uint32_t y = 0; y<worldsize; y++){
					for(uint32_t x = 0; x<worldsize; x++, i++){
						auto v = vaporPixels.data()[i];
						totalDensity += v;
						size_t idx = size_t(v * 99999.f);
						size_t idx_contrib = size_t(sqrt(sqrt(v)) * 99.f);
						if(idx<100){
							histogram[idx] += 1;
						}
						histogram_contribution[idx_contrib] += v;
						histogram_ranges[idx_contrib].add(glm::vec3(x,y,z));
					}
				}
			}

			auto totalSpan = glm::length(glm::vec3(worldsize, worldsize, worldsize));
			for(size_t i=0;i<histogram_ranges.size();++i){
				auto span3f = histogram_ranges[i].getSpan();
				auto span = glm::length(span3f)/totalSpan;
				auto contribution = histogram_contribution[i]/totalDensity;
				if(span>0){
					histogram_factor[i] = contribution / (span*span*span);
				}else if(contribution>0){
					histogram_factor[i] = 1;
				}
			}



			size_t i = 0;
			size_t total = 0;
			double total_pct = 0.0;
			cout << "histogram number of particles" << endl;
			for(auto & h: histogram){
				double pct = double(h)/double(vaporPixels.data().size());
				total_pct += pct;
				cout << i++ << ": " << pct << "%" << endl;
				total += h;
			}
			cout << "total " << vaporPixels.data().size() << " histogram total " << total << endl;
			cout << "total pct " << total_pct << endl;

			i = 0;
			total_pct = 0.0;
			cout << "histogram contribution" << endl;
			for(auto & h: histogram_contribution){
				double pct = h/totalDensity;
				total_pct += pct;
				cout << i++ << ": " << pct << "%" << endl;
			}
			cout << "total pct " << total_pct << endl;

			i = 0;
			total_pct = 0.0;
			cout << "histogram factor" << endl;
			for(auto & h: histogram_factor){
				cout << i++ << ": " << h << "%" << endl;
			}


			uint32_t level = log2(worldsize);
			cout << "storing with level " << level << endl;
			ofFile voxelsFile(voxelsFileName, ofFile::WriteOnly, true);
			std::vector<uint32_t> memVoxels;
			auto mask = worldsize-1;
			auto fToi32 = pow(2,32);
			for(uint32_t z = 0, i=0; z<worldsize; z++){
				for(uint32_t y = 0; y<worldsize; y++){
					for(uint32_t x = 0; x<worldsize; x++, i++){
						size_t idx_contrib = size_t(sqrt(sqrt(vaporPixels.data()[i])) * 99.f);
						auto factor = histogram_factor[idx_contrib];
						if(factor>0.01){
							uint32_t idx = 0;
							idx += (x<<(level*2));
							idx += (y<<level);
							idx += z;

							memVoxels.push_back(idx);
							memVoxels.push_back(vaporPixels.data()[i] * fToi32);
						}
					}
				}
			}
			voxelsFile.write((const char*)memVoxels.data(), memVoxels.size() * sizeof(uint32_t));
		}
	}

	//--------------------------------------------------------------
	void SnapshotRamses::setup(const std::string& folder, int frameIndex, float minDensity, float maxDensity, ofxTexture & volumeTexture, size_t worldsize)
	{
		clear();
		const float * data = nullptr;
		void * filedata = nullptr;
		size_t filedatalen = 0;
		auto rawFileName = ofFilePath::removeTrailingSlash(folder) + ".raw";
		auto particlesFileName = ofFilePath::removeTrailingSlash(folder) + ".particles";
		auto metaFileName = ofFilePath::removeTrailingSlash(folder) + "_meta.raw";
		auto voxelsFileName = ofFilePath::removeTrailingSlash(folder) + "_voxels.raw";
		auto particlesGroupsFileName = ofFilePath::removeTrailingSlash(folder) + ".groups";
		auto voxelsSize = 0;
		ofBufferObject particlesBuffer;
		ofBufferObject voxelsBuffer;
		std::vector<float> finalPixels;
		std::vector<size_t> particleGroups;
#if USE_VOXELS_COMPUTE_SHADER
		voxels2texture.setupShaderFromFile(GL_COMPUTE_SHADER, "shaders/voxels2texture3d.glsl");
		voxels2texture.linkProgram();
#endif

#if USE_PARTICLES_COMPUTE_SHADER
		particles2texture.setupShaderFromFile(GL_COMPUTE_SHADER, "shaders/particles2texture3d.glsl");
		particles2texture.linkProgram();
#endif
		
		if(!ofFile(rawFileName, ofFile::Reference).exists()){
			precalculate(folder, frameIndex, minDensity, maxDensity, worldsize);
		}


		// Load data
		{
			ofLogNotice() << "Loading from raw file";
			// Load metadata
			{
				auto then = ofGetElapsedTimeMicros();
				ofFile texture3dMeta(metaFileName, ofFile::ReadOnly, true);
				float min_density, max_density, min_size, max_size;
				glm::vec3 min_coords, max_coords, min_box, max_box;
				size_t numGroups;
				texture3dMeta >> min_density;
				texture3dMeta.ignore(1);
				texture3dMeta >> max_density;
				texture3dMeta.ignore(1);
				texture3dMeta >> min_coords;
				texture3dMeta.ignore(1);
				texture3dMeta >> max_coords;
				texture3dMeta.ignore(1);
				texture3dMeta >> min_size;
				texture3dMeta.ignore(1);
				texture3dMeta >> max_size;
				texture3dMeta.ignore(1);
				texture3dMeta >> min_box;
				texture3dMeta.ignore(1);
				texture3dMeta >> max_box;
				texture3dMeta.ignore(1);
				texture3dMeta >> m_numCells;
				texture3dMeta.ignore(1);
				texture3dMeta >> numGroups;
				m_densityRange.add(min_density);
				m_densityRange.add(max_density);
				m_coordRange.add(min_coords);
				m_coordRange.add(max_coords);
				m_sizeRange.add(min_size);
				m_sizeRange.add(max_size);
				m_boxRange = BoundingBox::fromMinMax(min_box, max_box);
				particleGroups.resize(numGroups);
				auto now = ofGetElapsedTimeMicros();
				cout << "time to load metadata " << float(now - then)/1000 << "ms." << endl;
			}

#if USE_VOXELS_COMPUTE_SHADER
			// Load voxels
			{
				voxelsSize = ofFile(voxelsFileName, ofFile::Reference).getSize();
				voxelsBuffer.allocate(voxelsSize, GL_STATIC_DRAW);
				auto then = ofGetElapsedTimeMicros();
				readToBuffer(voxelsFileName, voxelsSize, voxelsBuffer);
				auto now = ofGetElapsedTimeMicros();
				cout << "time to load voxels file " << float(now - then)/1000 << "ms. " <<
				        "for " << voxelsSize << " bytes" << endl;
			}
#elif USE_PARTICLES_COMPUTE_SHADER
			// Load particles
			{
				particlesBuffer.allocate(m_numCells*sizeof(Particle), GL_STATIC_DRAW);
				auto then = ofGetElapsedTimeMicros();
				readToBuffer(particlesFileName, m_numCells*sizeof(Particle), particlesBuffer);
				auto now = ofGetElapsedTimeMicros();
				cout << "time to load particles file " << float(now - then)/1000 << "ms. " <<
				        "for " << m_numCells << " particles" << endl;
			}

			// Load particle groups
			{
				auto then = ofGetElapsedTimeMicros();
				readToMemory(particlesGroupsFileName, particleGroups.size() * sizeof(size_t), (char*)particleGroups.data());
				auto now = ofGetElapsedTimeMicros();
				cout << "time to load groups file " << float(now - then)/1000 << "ms. " <<
				        "for " << particleGroups.size() << " groups" << endl;
			}
#else
			// Load raw texture
			{
				auto then = ofGetElapsedTimeMicros();
                #if defined(TARGET_LINUX) && FAST_READ
				    auto fd = open (ofToDataPath(rawFileName).c_str(), O_RDONLY, S_IRUSR);
					filedatalen = ofFile(rawFileName, ofFile::Reference).getSize();
					filedata = mmap (nullptr, filedatalen, PROT_READ, MAP_PRIVATE, fd, 0);
					if(filedata == (const void *)-1){
						ofLogError() << "Couldn't read memory mapped file";
						return;
					}
					//filedata += 16*sizeof(float) + 8;
					data = (const float*)filedata;
                #else
				    ofFile texture3dRaw(rawFileName, ofFile::ReadOnly, true);
					this->vaporPixelsBuffer = ofBuffer(texture3dRaw, 1024*1024*5);
					data = (float*)this->vaporPixelsBuffer.getData();
                #endif
				auto now = ofGetElapsedTimeMicros();
				cout << "time to load raw file " << float(now - then)/1000 << "ms." << endl;
			}
#endif
		}

		auto then = ofGetElapsedTimeMicros();

        #if USE_VOXELS_COMPUTE_SHADER
		    size_t numInstances = voxelsSize / (sizeof(uint32_t) * 2);
			ofTexture voxelsTexture;
			voxelsTexture.allocateAsBufferTexture(voxelsBuffer, GL_RG32I);
			cout << "dispatching compute shader with " << numInstances << " instances" << endl;
			voxels2texture.begin();
			volumeTexture.bindAsImage(0,GL_WRITE_ONLY,0,1,0);
			voxels2texture.setUniformTexture("voxels",voxelsTexture,0);
			voxels2texture.dispatchCompute(numInstances,1,1);
			voxels2texture.end();

			glBindImageTexture(0,0,0,0,0,GL_READ_WRITE,GL_R16F);
			//volumeTexture.copyTo(textureBuffer);
			//volumeTexture.loadData(textureBuffer, GL_RED);
        #elif USE_PARTICLES_COMPUTE_SHADER
		    ofTexture particlesTexture;
			particlesTexture.allocateAsBufferTexture(particlesBuffer, GL_RGBA32F);
			glm::vec3 coordSpan = m_boxRange.getSpan();
			auto normalizeFactor = std::max(std::max(coordSpan.x, coordSpan.y), coordSpan.z);
			auto scale = worldsize / normalizeFactor;
			auto offset = -m_boxRange.min;

			particles2texture.begin();
			particles2texture.setUniformTexture("particles",particlesTexture,0);
			volumeTexture.bindAsImage(0,GL_READ_WRITE,0,1,0);
			particles2texture.setUniform1f("size",worldsize);
			particles2texture.setUniform1f("minDensity",minDensity * m_densityRange.getMin());
			particles2texture.setUniform1f("maxDensity",maxDensity * m_densityRange.getMax());
			particles2texture.setUniform3f("minBox",m_boxRange.min);
			particles2texture.setUniform3f("maxBox",m_boxRange.max);
			particles2texture.setUniform3f("boxSpan",m_boxRange.getSpan());
			particles2texture.setUniform1f("scale", scale);
			particles2texture.setUniform3f("offset", offset);

			size_t idx_offset = 0;
			for(auto next: particleGroups){
				particles2texture.setUniform1f("idx_offset", idx_offset);
				particles2texture.setUniform1f("next", next);
				particles2texture.dispatchCompute((next-idx_offset)/256+1,1,1);
				idx_offset = next;
				//glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );
			}
			particles2texture.end();
			glBindImageTexture(0,0,0,0,0,GL_READ_WRITE,GL_R16F);
        #else

            #if READ_TO_BUFFER
		        ofBufferObject textureBuffer;
				textureBuffer.allocate(worldsize*worldsize*worldsize*4, GL_STATIC_DRAW);
				char * gpudata = (char *)textureBuffer.map(GL_WRITE_ONLY);
				memcpy(gpudata, data, worldsize*worldsize*worldsize*4);
				textureBuffer.unmap();
				volumeTexture.loadData(textureBuffer, GL_RED);
            #else
		        volumeTexture.loadData(data, worldsize, worldsize, worldsize, 0, 0, 0, GL_RED);
            #endif
        #endif

		volumeTexture.generateMipmaps();
		volumeTexture.setMinMagFilters(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
		//volumeTexture.setMinMagFilters(GL_LINEAR, GL_LINEAR);

		auto now = ofGetElapsedTimeMicros();
		cout << "time to load texture and generate mipmaps " << float(now - then)/1000 << "ms." << endl;

		cout << "loaded " << m_numCells << " particles";

        #if defined(TARGET_LINUX) && FAST_READ
		    if(filedata!=nullptr){
				munmap(filedata, filedatalen);
			}
        #endif

		/*GLint num_textures;
		glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &num_textures);
		std::vector<GLint> formats(num_textures);
		glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, formats.data());
		for(auto format: formats){
			cout << format << endl;
		}*/

		m_vboMesh.setVertexBuffer(particlesBuffer, 4, sizeof(Particle), 0);
		m_vboMesh.setAttributeBuffer(SIZE_ATTRIBUTE, particlesBuffer, 1, sizeof(Particle), sizeof(float)*4);
		m_vboMesh.setAttributeBuffer(DENSITY_ATTRIBUTE, particlesBuffer, 1, sizeof(Particle), sizeof(float)*5);

		m_bLoaded = true;
	}
	
	//--------------------------------------------------------------
	void SnapshotRamses::clear()
	{
		m_coordRange.clear();
		m_sizeRange.clear();
		m_densityRange.clear();

		m_numCells = 0;

		m_bLoaded = false;
	}

	//--------------------------------------------------------------
	void SnapshotRamses::loadhdf5(const std::string& file, std::vector<float>& elements)
	{
		ofxHDF5File h5File;
		h5File.open(file, true);
		ofLogVerbose() << "File '" << file << "' has " << h5File.getNumDataSets() << " datasets";

		for (int i = 0; i < h5File.getNumDataSets(); ++i) 
		{
			ofLogVerbose("SnapshotRamses::load") << "  DataSet " << i << ": " << h5File.getDataSetName(i);
		}
		string dataSetName = h5File.getDataSetName(0);
		ofxHDF5DataSetPtr dataSet = h5File.loadDataSet(dataSetName);

		int count = dataSet->getDimensionSize(0);
		dataSet->setHyperslab(0, count);
		elements.resize(count);

		// Data is 64-bit, read it directly (losing precision).
		dataSet->read(elements.data(), H5_DATATYPE_FLOAT);
	}

	//--------------------------------------------------------------
	void SnapshotRamses::update(ofShader& shader)
	{

	}

	//--------------------------------------------------------------
	void SnapshotRamses::draw()
	{
		m_vboMesh.draw(GL_POINTS, 0, m_numCells);
	}

	//--------------------------------------------------------------
	void SnapshotRamses::drawOctree(float minDensity, float maxDensity){
		vaporOctree.drawLeafs(minDensity, maxDensity);
	}

	//--------------------------------------------------------------
	ofxRange3f& SnapshotRamses::getCoordRange()
	{
		return m_coordRange;
	}
	
	//--------------------------------------------------------------
	ofxRange1f& SnapshotRamses::getSizeRange()
	{
		return m_sizeRange;
	}
	
	//--------------------------------------------------------------
	ofxRange1f& SnapshotRamses::getDensityRange()
	{
		return m_densityRange;
	}

	//--------------------------------------------------------------
	std::size_t SnapshotRamses::getNumCells() const
	{
		return m_numCells;
	}

	//--------------------------------------------------------------
	bool SnapshotRamses::isLoaded() const
	{
		return m_bLoaded;
	}
}
