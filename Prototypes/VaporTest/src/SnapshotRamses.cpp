#include "SnapshotRamses.h"

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
	void SnapshotRamses::setup(const std::string& folder, int frameIndex)
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

		m_numCells = posX.size();

		// Set the ranges for all data.
		for (size_t i = 0; i < posX.size(); ++i)
		{
            m_coordRange.add(glm::vec3(posX[i], posY[i], posZ[i]));
			m_sizeRange.add(cellSize[i]);
			m_densityRange.add(density[i]);
		}
		auto min = m_coordRange.getMin();
		auto max = m_coordRange.getMax();
		cout << min.x << ", " << min.y << ", " << min.z << " - " << max.x << ", " << max.y << ", " << max.z << endl;

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

		// Set up the VBO.
		//m_vboMesh = ofMesh::box(1, 1, 1, 1, 1, 1);
		m_vboMesh.addVertex({0,0,0});
		m_vboMesh.setMode(OF_PRIMITIVE_POINTS);
		m_vboMesh.setUsage(GL_STATIC_DRAW);

		// Upload per-instance data to the VBO.
		m_vboMesh.getVbo().setAttributeData(DENSITY_ATTRIBUTE, density.data(), 1, density.size(), GL_STATIC_DRAW, 0);
		m_vboMesh.getVbo().setAttributeDivisor(DENSITY_ATTRIBUTE, 1);

		std::vector<ofVec4f> transforms;
		transforms.resize(posX.size());
		for (size_t i = 0; i < transforms.size(); ++i) 
		{
			transforms[i] = ofVec4f(posX[i], posY[i], posZ[i], cellSize[i]);
		}
		m_vboMesh.getVbo().setAttributeData(POSITION_SIZE_ATTRIBUTE, (float*)transforms.data(), 4, transforms.size(), GL_STATIC_DRAW, 0);
		m_vboMesh.getVbo().setAttributeDivisor(POSITION_SIZE_ATTRIBUTE, 1);

		// Upload per-instance transform data to the TBO.
		/*m_bufferObject.allocate();
		m_bufferObject.bind(GL_TEXTURE_BUFFER);
		m_bufferObject.setData(transforms, GL_STREAM_DRAW);

		m_bufferTexture.allocateAsBufferTexture(m_bufferObject, GL_RGBA32F);*/

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
		//shader.setUniformTexture("uTransform", m_bufferTexture, 0);
	}

	//--------------------------------------------------------------
	void SnapshotRamses::draw()
	{
		m_vboMesh.drawInstanced(OF_MESH_FILL, m_numCells);
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
