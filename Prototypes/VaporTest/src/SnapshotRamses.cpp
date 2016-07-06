#include "SnapshotRamses.h"
#include "ThreadPool.h"

namespace ent
{
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

	//--------------------------------------------------------------
	void SnapshotRamses::setup(const std::string& folder, int frameIndex, float minDensity, float maxDensity, ofxTexture & volumeTexture, size_t octree_size)
	{
		clear();
		
		// Load the HDF5 data.
		std::vector<float> posX;
		std::vector<float> posY;
		std::vector<float> posZ;
		std::vector<float> cellSize;
		std::vector<float> density;

		load(folder + "x/seq_" + ofToString(frameIndex) + "_x.h5", posX);
		load(folder + "y/seq_" + ofToString(frameIndex) + "_y.h5", posY);
		load(folder + "z/seq_" + ofToString(frameIndex) + "_z.h5", posZ);
		load(folder + "dx/seq_" + ofToString(frameIndex) + "_dx.h5", cellSize);
		load(folder + "density/seq_" + ofToString(frameIndex) + "_density.h5", density);

		std::vector<Particle> particles;
		/*for(size_t i=0;i<posX.size();++i){
			particles[i] = {{posX[i], posY[i], posZ[i]}, cellSize[i], density[i]};
		}*/

		m_numCells = posX.size();

		// Set the ranges for all data.
		for (size_t i = 0; i < posX.size(); ++i)
		{
			m_densityRange.add(density[i]);
			m_coordRange.add(glm::vec3(posX[i], posY[i], posZ[i]));
			m_sizeRange.add(cellSize[i]);
		}

		auto threshold = minDensity * m_densityRange.getMin() + (maxDensity * m_densityRange.getMax() - minDensity * m_densityRange.getMin()) * 0.0002f;
		for (size_t i=0; i < posX.size(); ++i){
			if(density[i]>threshold){
				particles.push_back({{posX[i], posY[i], posZ[i]}, cellSize[i], density[i]});
			}
		}

		//ThreadPool::pool().addTask([]{cout << "starting pool" << endl;});
		auto then = ofGetElapsedTimeMicros();
		this->octree.setup(particles);
		this->octree.compute(octree_size, minDensity * m_densityRange.getMin(), maxDensity * m_densityRange.getMax(), m_coordRange);
		auto now = ofGetElapsedTimeMicros();
		cout << "time to compute octree " << float(now - then)/1000 << "ms. size: " << octree.size() << endl;
		auto min = m_coordRange.getMin();
		auto max = m_coordRange.getMax();
		cout << min.x << ", " << min.y << ", " << min.z << " - " << max.x << ", " << max.y << ", " << max.z << endl;
		cout << "size range " << m_sizeRange.getMin() << " - " << m_sizeRange.getMax() << endl;

		// Expand coord range taking cell size into account.
		m_coordRange.add(m_coordRange.getMin() - m_sizeRange.getMax());
		m_coordRange.add(m_coordRange.getMax() + m_sizeRange.getMax());

		// Find the dimension with the max span, and set all spans to be the same (since we're rendering a cube).
		glm::vec3 coordSpan = m_coordRange.getSpan();
		float maxSpan = MAX(coordSpan.x, MAX(coordSpan.y, coordSpan.z));
		glm::vec3 spanOffset(maxSpan * 0.5);
		glm::vec3 coordMid = m_coordRange.getCenter();
		m_coordRange.add(coordMid - spanOffset);
		m_coordRange.add(coordMid + spanOffset);

		/*auto octree_particles = octree.toVector();
		m_numCells = octree_particles.size();
		cout << "octree num particles " << m_numCells << endl;
		ofBufferObject particlesBuffer;
		particlesBuffer.allocate();
		particlesBuffer.setData(octree_particles, GL_STATIC_DRAW);
		m_vboMesh.setVertexBuffer(particlesBuffer, 3, sizeof(Particle), 0);
		m_vboMesh.setAttributeBuffer(SIZE_ATTRIBUTE, particlesBuffer, 1, sizeof(Particle), sizeof(float)*3);
		m_vboMesh.setAttributeBuffer(DENSITY_ATTRIBUTE, particlesBuffer, 1, sizeof(Particle), sizeof(float)*4);*/


		/*for(size_t z=0;z<octree.size();++z){
			cout << z << endl;
			pixels.push_back(std::move(octree.getPixels(z, m_densityRange.getMin(), m_densityRange.getMax())));
		}
		tex.allocate(pixels[0]);
		pixelsIdx = 0;*/


		std::vector<float> data;
		for(size_t z=0;z<octree_size;++z){
			cout << z << endl;
			auto pixels = octree.getPixels(z, minDensity * m_densityRange.getMin(), maxDensity * m_densityRange.getMax());
			data.insert(data.end(), pixels.begin(), pixels.end());
		}

		volumeTexture.loadData(data.data(), octree_size, octree_size, octree_size, 0, 0, 0, GL_RED);


		m_bLoaded = true;
	}
	
	//--------------------------------------------------------------
	void SnapshotRamses::clear()
	{
		m_bufferTexture.clear();

		m_coordRange.clear();
		m_sizeRange.clear();
		m_densityRange.clear();

		m_numCells = 0;

		m_bLoaded = false;
	}

	//--------------------------------------------------------------
	void SnapshotRamses::load(const std::string& file, std::vector<float>& elements)
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
		octree.drawLeafs(minDensity, maxDensity);
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
