#include "SnapshotRamses.h"
#include "Constants.h"
#include <numeric>
#include "H5Cpp.h"
#include <curl/curl.h>

#ifdef TARGET_LINUX
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

//#include "turbojpeg.h"

namespace ent
{


	void sftpDownload(const std::string & from, const std::string & to){
		auto startServer = from.find("@");
		auto server = from.substr(0, from.find(":",startServer));
		auto file = from.substr(from.find(":",startServer) + 1, from.npos);
		auto localdir = ofFilePath::getEnclosingDirectory(to);

		auto command = "cd " + localdir + "; lftp -c 'open " + server + " && get " + file + "'";
		cout << command << endl;
		ofSystem(command);
	}

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
		//------------------------------------
		// Load the HDF5 data.
		std::vector<float> posX;
		std::vector<float> posY;
		std::vector<float> posZ;
		std::vector<float> cellSize;
		std::vector<float> density;

		auto then = ofGetElapsedTimeMicros();
		loadhdf5(folder + "/hdf5/"  +  getXHDF5Path(frameIndex), posX);
		loadhdf5(folder + "/hdf5/"  +  getYHDF5Path(frameIndex), posY);
		loadhdf5(folder + "/hdf5/"  +  getZHDF5Path(frameIndex), posZ);
		loadhdf5(folder + "/hdf5/"  +  getDXHDF5Path(frameIndex), cellSize);
		loadhdf5(folder + "/hdf5/"  +  getDensityHDF5Path(frameIndex), density);
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

			if(firstFrame){
				m_densityRange.clear();
				m_coordRange.clear();
				m_sizeRange.clear();

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
			firstFrame = false;
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


		//------------------------------------
		// Write data to files
		{
			#if USE_RAW
			ofFile texture3dRaw(rawFileName, ofFile::WriteOnly, true);
			texture3dRaw.write((const char*)this->vaporPixels.data().data(), this->vaporPixels.data().size() * sizeof(float));
			#endif

			ofFile texture3dMeta(metaFileName, ofFile::WriteOnly, true);
			texture3dMeta << m_densityRange.getMin() << " " << m_densityRange.getMax() << " " <<
							m_coordRange.getMin() << " " << m_coordRange.getMax() << " " <<
							m_sizeRange.getMin() << " " << m_sizeRange.getMax() << " " <<
							m_boxRange.min << " " << m_boxRange.max << " " <<
							vaporPixels.getParticlesInBox().size() << " " <<
							vaporPixels.getGroupIndices().size();


			#if USE_PARTICLES_COMPUTE_SHADER
			ofFile particlesFile(particlesFileName, ofFile::WriteOnly, true);
			#if USE_HALF_PARTICLE
				particlesFile.write((const char*)vaporPixels.getHalfParticlesInBox().data(), vaporPixels.getHalfParticlesInBox().size() * sizeof(HalfParticle));
			#else
				particlesFile.write((const char*)vaporPixels.getParticlesInBox().data(), vaporPixels.getParticlesInBox().size() * sizeof(Particle));
			#endif
			ofFile particlesGroupsFile(particlesGroupsFileName, ofFile::WriteOnly, true);
			particlesGroupsFile.write((const char*)vaporPixels.getGroupIndices().data(), sizeof(size_t) * vaporPixels.getGroupIndices().size());
			#endif
		}



		//------------------------------------
		// Find out voxels that contribute more
		// to the final texture and remove the rest
		// as a form of lossy compression
		#if USE_VOXELS_COMPUTE_SHADER


		#if USE_VOXELS_DCT_COMPRESSION


		ofFloatPixels pixels;
		pixels.allocate(512*16, 512*32, OF_PIXELS_GRAY);
		auto ptr = vaporPixels.data().data();
		for(size_t z=0, x=0, y=0; z<worldsize; ++z){
			ofFloatPixels pixelsSlice;
			pixelsSlice.setFromExternalPixels(ptr,worldsize,worldsize,OF_PIXELS_GRAY);
			ofSaveImage(pixelsSlice, voxelsFileName + "_" + ofToString(z) + ".tif");
			//pixelsSlice.pasteInto(pixels,x,y);
			ptr+=worldsize*worldsize;
			/*x+=worldsize;
			if(x>pixels.getWidth()){
				x=0;
				y+=worldsize;
			}*/
		}



		#else



		{
			const size_t numBins = 500;
			std::array<size_t,numBins> histogram = {{0}};
			std::array<double,numBins> histogram_contribution = {{0.}};
			std::array<size_t,numBins> histogram_num_particles = {{0}};
			ofxRange3f zero;
			zero.clear();
			std::array<ofxRange3f,numBins> histogram_ranges = {{ zero }};
			std::array<float,numBins> histogram_factor = {{ 0.f }};
			double totalDensity = 0.;

			for(uint32_t z = 0, i=0; z<worldsize; z++){
				for(uint32_t y = 0; y<worldsize; y++){
					for(uint32_t x = 0; x<worldsize; x++, i++){
						auto v = vaporPixels.data()[i];
						totalDensity += v;
						size_t idx = size_t(v * 99999.f);
						size_t idx_contrib = size_t(sqrt(sqrt(v)) * float(numBins-1));
						if(idx<numBins){
							histogram[idx] += 1;
						}
						histogram_contribution[idx_contrib] += v;
						histogram_ranges[idx_contrib].add(glm::vec3(x,y,z));
						histogram_num_particles[idx_contrib] += 1;
					}
				}
			}

			auto totalSpan = glm::length(glm::vec3(worldsize, worldsize, worldsize));
			for(size_t i=0;i<histogram_ranges.size();++i){
				auto span3f = histogram_ranges[i].getSpan();
				auto span = glm::length(span3f)/totalSpan;
				auto contribution = histogram_contribution[i]/totalDensity;
				if(span>0){
					histogram_factor[i] = contribution / pow(span,5);
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



			std::array<std::pair<float,size_t>,numBins> histogram_factor_sorted;
			for(size_t i=0;i<histogram_factor.size();i++){
				histogram_factor_sorted[i] = std::make_pair(histogram_factor[i], histogram_num_particles[i]);
			}
			std::sort(histogram_factor_sorted.begin(), histogram_factor_sorted.end(), [&](const std::pair<float,size_t> & p1, const std::pair<float,size_t> & p2){
				return p1.first > p2.first;
			});
			i = 0;
			total_pct = 0.0;
			double totalFactor = 0;
			size_t byteRate = 100*1024*1024; //800Mb/s @ 30fps
			size_t accumulatedSize = 0;
			float threshold=0;
			size_t sizeBeforeThreshold = 0;
			cout << "histogram factor" << endl;
			for(auto & h: histogram_factor_sorted){
				auto binSize = h.second * sizeof(int32_t)*2;
				accumulatedSize+=binSize;
				if(accumulatedSize>byteRate && threshold==0){
					threshold = h.first;
					cout << "found threshold at " << threshold << endl;
				}else if(threshold == 0){
					sizeBeforeThreshold += binSize;
				}
				totalFactor+=h.first;
				cout << i++ << ": " << h.first*100 << "% " << binSize/1024. << "KB / " << accumulatedSize/1024./1024. << "MB" << endl;
			}


			uint32_t level = log2(worldsize);
			cout << "storing with level " << level << endl;
			ofFile voxelsFile(voxelsFileName, ofFile::WriteOnly, true);
			std::vector<uint32_t> memVoxels;
			auto mask = worldsize-1;
			auto fToi32 = pow(2,32);
			size_t particlesInThresholdBin = (byteRate - sizeBeforeThreshold)/sizeof(int32_t)*2;
			for(uint32_t z = 0, i=0; z<worldsize; z++){
				for(uint32_t y = 0; y<worldsize; y++){
					for(uint32_t x = 0; x<worldsize; x++, i++){
						size_t idx_contrib = size_t(sqrt(sqrt(vaporPixels.data()[i])) * float(numBins-1));
						auto factor = histogram_factor[idx_contrib];
						if(factor>threshold){
							uint32_t idx = 0;
							idx += (x<<(level*2));
							idx += (y<<level);
							idx += z;

							memVoxels.push_back(idx);
							memVoxels.push_back(vaporPixels.data()[i] * fToi32);
						}else if(threshold==factor && particlesInThresholdBin>0){
							particlesInThresholdBin-=1;
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
			cout << "number of voxels " << memVoxels.size()/2 << endl;
			if(memVoxels.size()*sizeof(int32_t)>128*1024*1024){
				cout << "Couldn't compress texture to less than " << 128 << " MB" << endl;
				std::exit(0);
			}
			if(memVoxels.size()==0){
				cout << "No voxels are above the compression threshold" << endl;
			}
		}
		#endif
		#endif
	}

	//--------------------------------------------------------------
	void SnapshotRamses::setup(Settings & settings)
	{
		const float * data = nullptr;
		void * filedata = nullptr;
		size_t filedatalen = 0;
		metaFileName = ofFilePath::removeTrailingSlash(settings.folder) + "/" + ofToString(settings.frameIndex) + "_meta.raw";
		rawFileName = ofFilePath::removeTrailingSlash(settings.folder) + "/" + ofToString(settings.frameIndex) + ".raw";
		particlesFileName = ofFilePath::removeTrailingSlash(settings.folder) + "/" + ofToString(settings.frameIndex) +".particles";
		voxelsFileName = ofFilePath::removeTrailingSlash(settings.folder) + "/" + ofToString(settings.frameIndex) + "_voxels.raw";
		particlesGroupsFileName = ofFilePath::removeTrailingSlash(settings.folder) + "/" + ofToString(settings.frameIndex) + ".groups";

		clear();


		cout << "metadata " << metaFileName << endl;
		auto voxelsSize = 0;
		std::vector<size_t> particleGroups;

		auto metadataPrecalculated = ofFile(metaFileName, ofFile::Reference).exists();
		auto particlesPrecalculated = ofFile(particlesFileName, ofFile::Reference).exists();
		auto voxelsPrecalculated = ofFile(voxelsFileName, ofFile::Reference).exists();
		auto rawPrecalculated = ofFile(rawFileName, ofFile::Reference).exists();

		if(settings.urlFolder!=""){
			bool needsDownload = false;

			needsDownload |= !metadataPrecalculated;
#if USE_PARTICLES_COMPUTE_SHADER
			needsDownload |= particlesPrecalculated;
#endif
#if USE_VOXELS_COMPUTE_SHADER
			needsDownload |= voxelsPrecalculated;
#endif
#if USE_RAW
			needsDownload |= rawPrecalculated;
#endif

			if(!ofFile(settings.folder + "/hdf5/" + getXHDF5Path(settings.frameIndex), ofFile::Reference).exists()){
				sftpDownload(settings.urlFolder + "/" + getXHDF5Path(settings.frameIndex), settings.folder + "/hdf5/" + getXHDF5Path(settings.frameIndex));
			}
			if(!ofFile(settings.folder + "/hdf5/" + getYHDF5Path(settings.frameIndex), ofFile::Reference).exists()){
				sftpDownload(settings.urlFolder + "/" + getYHDF5Path(settings.frameIndex), settings.folder + "/hdf5/" + getYHDF5Path(settings.frameIndex));
			}
			if(!ofFile(settings.folder + "/hdf5/" + getZHDF5Path(settings.frameIndex), ofFile::Reference).exists()){
				sftpDownload(settings.urlFolder + "/" + getZHDF5Path(settings.frameIndex), settings.folder + "/hdf5/" + getZHDF5Path(settings.frameIndex));
			}
			if(!ofFile(settings.folder + "/hdf5/" + getDXHDF5Path(settings.frameIndex), ofFile::Reference).exists()){
				sftpDownload(settings.urlFolder + "/" + getDXHDF5Path(settings.frameIndex), settings.folder + "/hdf5/" + getDXHDF5Path(settings.frameIndex));
			}
			if(!ofFile(settings.folder + "/hdf5/" + getDensityHDF5Path(settings.frameIndex), ofFile::Reference).exists()){
				sftpDownload(settings.urlFolder + "/" + getDensityHDF5Path(settings.frameIndex), settings.folder + "/hdf5/" + getDensityHDF5Path(settings.frameIndex));
			}
		}

		if(metadataPrecalculated){
			auto then = ofGetElapsedTimeMicros();
			ofFile texture3dMeta(metaFileName, ofFile::ReadOnly, true);
			glm::vec3 min_coords, max_coords, min_box, max_box;
			float min_density, max_density, min_size, max_size;
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
			if(firstFrame){
				m_densityRange.add(min_density);
				m_densityRange.add(max_density);
				m_coordRange.add(min_coords);
				m_coordRange.add(max_coords);
				m_sizeRange.add(min_size);
				m_sizeRange.add(max_size);
				m_boxRange = BoundingBox::fromMinMax(min_box, max_box);
			}
			particleGroups.resize(numGroups);
			auto now = ofGetElapsedTimeMicros();
			cout << "time to load metadata " << float(now - then)/1000 << "ms." << endl;

//			if(min_density != settings.minDensity || max_density != settings.maxDensity){
//				ofFile(rawFileName, ofFile::Reference).remove();
//				ofFile(particlesFileName, ofFile::Reference).remove();
//				ofFile(metaFileName, ofFile::Reference).remove();
//				ofFile(voxelsFileName, ofFile::Reference).remove();
//				ofFile(particlesGroupsFileName, ofFile::Reference).remove();

//				metadataPrecalculated = false;
//				particlesPrecalculated = false;
//				voxelsPrecalculated = false;
//				rawPrecalculated = false;
//			}
		}

		auto needsPrecalculate = !metadataPrecalculated;
#if USE_PARTICLES_COMPUTE_SHADER
		needsPrecalculate |= !particlesPrecalculated;
#endif
#if USE_VOXELS_COMPUTE_SHADER
		needsPrecalculate |= !voxelsPrecalculated;
#endif
#if USE_RAW
		needsPrecalculate |= !rawPrecalculated;
#endif
#if HDF5_DIRECT
		needsPrecalculate = true;
#endif


		if(needsPrecalculate){
			precalculate(settings.folder, settings.frameIndex, settings.minDensity, settings.maxDensity, settings.worldsize);
			auto then = ofGetElapsedTimeMicros();
			ofFile texture3dMeta(metaFileName, ofFile::ReadOnly, true);
			glm::vec3 min_coords, max_coords, min_box, max_box;
			float min_density, max_density, min_size, max_size;
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
			if(firstFrame){
				m_densityRange.add(min_density);
				m_densityRange.add(max_density);
				m_coordRange.add(min_coords);
				m_coordRange.add(max_coords);
				m_sizeRange.add(min_size);
				m_sizeRange.add(max_size);
				m_boxRange = BoundingBox::fromMinMax(min_box, max_box);
			}
			particleGroups.resize(numGroups);
			auto now = ofGetElapsedTimeMicros();
			cout << "time to load metadata " << float(now - then)/1000 << "ms." << endl;
		}


		// Load data from precalculated files
		{

#if USE_VOXELS_COMPUTE_SHADER
			// Load voxels
			{
				int maxBufferTextureSize;
				glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &maxBufferTextureSize);
				voxelsSize = std::min(ofFile(voxelsFileName, ofFile::Reference).getSize(), (uint64_t)maxBufferTextureSize);
				auto then = ofGetElapsedTimeMicros();
				readToBuffer(voxelsFileName, voxelsSize, settings.voxelsBuffer);
				auto now = ofGetElapsedTimeMicros();
				cout << "time to load voxels file " << float(now - then)/1000 << "ms. " <<
						"for " << voxelsSize << " bytes" << endl;
			}
#elif USE_PARTICLES_COMPUTE_SHADER || USE_VBO
			// Load particles
			{
				auto then = ofGetElapsedTimeMicros();
				#if USE_HALF_PARTICLE
					readToBuffer(particlesFileName, m_numCells*sizeof(HalfParticle), settings.particlesBuffer);
				#else
					readToBuffer(particlesFileName, m_numCells*sizeof(Particle), settings.particlesBuffer);
				#endif
				auto now = ofGetElapsedTimeMicros();
				cout << "time to load particles file " << float(now - then)/1000 << "ms. " <<
						"for " << m_numCells << " particles" << endl;
			}
	#if USE_PARTICLES_COMPUTE_SHADER
			// Load particle groups
			{
				auto then = ofGetElapsedTimeMicros();
				readToMemory(particlesGroupsFileName, particleGroups.size() * sizeof(size_t), (char*)particleGroups.data());
				auto now = ofGetElapsedTimeMicros();
				cout << "time to load groups file " << float(now - then)/1000 << "ms. " <<
						"for " << particleGroups.size() << " groups" << endl;
			}
	#endif
#elif HDF5_DIRECT
			{
				data = vaporPixels.data().data();
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
					this->vaporPixelsBuffer = ofBuffer(texture3dRaw, settings.worldsize * settings.worldsize * settings.worldsize * sizeof(float));
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
			cout << "dispatching compute shader with " << numInstances << " instances" << endl;
			settings.voxels2texture.begin();
			settings.voxels2texture.setUniformTexture("voxels",settings.voxelsTexture,0);
			settings.volumeTexture.bindAsImage(0,GL_WRITE_ONLY,0,1,0);
			settings.voxels2texture.dispatchCompute(numInstances,1,1);
			settings.voxels2texture.setUniform1f("numVoxels", numInstances);
			settings.voxels2texture.end();
			glBindImageTexture(0,0,0,0,0,GL_READ_WRITE,GL_R16F);
			glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );

		#elif USE_PARTICLES_COMPUTE_SHADER
			glm::vec3 coordSpan = m_boxRange.getSpan();
			auto normalizeFactor = std::max(std::max(coordSpan.x, coordSpan.y), coordSpan.z);
			auto scale = settings.worldsize / normalizeFactor;
			auto offset = -m_boxRange.min;

			settings.particles2texture.begin();
			settings.particles2texture.setUniformTexture("particles",settings.particlesTexture,0);
			settings.volumeTexture.bindAsImage(0,GL_READ_WRITE,0,1,0);
			settings.particles2texture.setUniform1f("size",settings.worldsize);
			settings.particles2texture.setUniform1f("minDensity",settings.minDensity * m_densityRange.getMin());
			settings.particles2texture.setUniform1f("maxDensity",settings.maxDensity * m_densityRange.getMax());
			settings.particles2texture.setUniform3f("minBox",m_boxRange.min);
			settings.particles2texture.setUniform3f("maxBox",m_boxRange.max);
			settings.particles2texture.setUniform3f("boxSpan",m_boxRange.getSpan());
			settings.particles2texture.setUniform1f("scale", scale);
			settings.particles2texture.setUniform3f("offset", offset);

			size_t idx_offset = 0;
			for(auto next: particleGroups){
				settings.particles2texture.setUniform1f("idx_offset", idx_offset);
				settings.particles2texture.setUniform1f("next", next);
				settings.particles2texture.dispatchCompute((next-idx_offset)/256+1,1,1);
				idx_offset = next;
				glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );
			}
			settings.particles2texture.end();
			glBindImageTexture(0,0,0,0,0,GL_READ_WRITE,GL_R16F);
		#else

			#if READ_TO_BUFFER
				ofBufferObject textureBuffer;
				textureBuffer.allocate(settings.worldsize*settings.worldsize*settings.worldsize*4, GL_STATIC_DRAW);
				char * gpudata = (char *)textureBuffer.map(GL_WRITE_ONLY);
				memcpy(gpudata, data, settings.worldsize*settings.worldsize*settings.worldsize*4);
				textureBuffer.unmap();
				settings.volumeTexture.loadData(textureBuffer, GL_RED);
			#else
				settings.volumeTexture.loadData(data, settings.worldsize, settings.worldsize, settings.worldsize, 0, 0, 0, GL_RED);
			#endif
		#endif

		#if USE_TEXTURE_3D_MIPMAPS
			settings.volumeTexture.generateMipmaps();
			settings.volumeTexture.setMinMagFilters(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
		#else
			settings.volumeTexture.setMinMagFilters(GL_LINEAR, GL_LINEAR);
		#endif

		auto now = ofGetElapsedTimeMicros();
		cout << "time to load texture and generate mipmaps " << float(now - then)/1000 << "ms." << endl;

		cout << "loaded " << m_numCells << " particles" << endl;

		#if defined(TARGET_LINUX) && FAST_READ && USE_RAW
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
		#if USE_VBO
			m_vboMesh.setAttributeBuffer(POSITION_ATTRIBUTE, settings.particlesBuffer, 4, sizeof(Particle), 0);
			m_vboMesh.setAttributeBuffer(SIZE_ATTRIBUTE, settings.particlesBuffer, 1, sizeof(Particle), sizeof(float)*4);
			m_vboMesh.setAttributeBuffer(DENSITY_ATTRIBUTE, settings.particlesBuffer, 1, sizeof(Particle), sizeof(float)*5);
		#endif


#if USE_PARTICLES_COMPUTE_SHADER
		{
			vector<Particle> particles;
			auto pptr = settings.particlesBuffer.map<Particle>(GL_READ_ONLY);
			particles.assign(pptr, pptr + m_numCells);
			settings.particlesBuffer.unmap();


			auto then = ofGetElapsedTimeMicros();
			cout << "calculating octree with " << log2(settings.worldsize*2) << " levels" << endl;
			this->vaporOctree.setup(particles);
			this->vaporOctree.compute(log2(settings.worldsize*2), settings.minDensity * m_densityRange.getMin(), settings.maxDensity * m_densityRange.getMin());//settings.maxDensity * m_densityRange.getMax());
			auto now = ofGetElapsedTimeMicros();
			cout << "time to compute octree " << float(now - then)/1000 << "ms." << endl;
			cout << "octree maxlevel: " << vaporOctree.getMaxLevel() << endl;
		}
#endif

		m_bLoaded = true;
	}

	//--------------------------------------------------------------
	void SnapshotRamses::clear()
	{
		if(firstFrame){
			m_coordRange.clear();
			m_sizeRange.clear();
			m_densityRange.clear();
		}

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

	void SnapshotRamses::drawOctreeDensities(const ofTrueTypeFont & ttf, const ofCamera & camera, const glm::mat4 & model, float minDensity, float maxDensity){
		vaporOctree.drawDensities(ttf, camera, model, minDensity, maxDensity);
	}

	//--------------------------------------------------------------
	ofMesh SnapshotRamses::getOctreeMesh(float minDensity, float maxDensity) const{
		return vaporOctree.getMesh(minDensity, maxDensity, VaporOctree::SizeSmallerFirst);
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

	//--------------------------------------------------------------
	std::string SnapshotRamses::getXHDF5Path(int frameIndex){
		return "x_nout=" + ofToString(frameIndex,0,4,'0') + "_hdf5.h5";
	}

	//--------------------------------------------------------------
	std::string SnapshotRamses::getYHDF5Path(int frameIndex){
		return "y_nout=" + ofToString(frameIndex,0,4,'0') + "_hdf5.h5";
	}

	//--------------------------------------------------------------
	std::string SnapshotRamses::getZHDF5Path(int frameIndex){
		return "z_nout=" + ofToString(frameIndex,0,4,'0') + "_hdf5.h5";
	}

	//--------------------------------------------------------------
	std::string SnapshotRamses::getDensityHDF5Path(int frameIndex){
		return "density_nout=" + ofToString(frameIndex,0,4,'0') + "_hdf5.h5";
	}

	//--------------------------------------------------------------
	std::string SnapshotRamses::getDXHDF5Path(int frameIndex){
		return "dx_nout=" + ofToString(frameIndex,0,4,'0') + "_hdf5.h5";

	}
}
