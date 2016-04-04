#pragma once

#include "ofMain.h"
#include "ofxHDF5.h"
#include "ofxRange.h"

namespace ent
{
	enum SnapshotAttributes
	{
		DENSITY_ATTRIBUTE = 5
	};
	
	class SnapshotRamses
	{
	public:
		SnapshotRamses();
		~SnapshotRamses();

		void setup(const std::string& folder);
		void clear();

		void update(ofShader& shader);
		void draw();

		ofxRange3f& getCoordRange();
		ofxRange1f& getSizeRange();
		ofxRange1f& getDensityRange();

		std::size_t getNumCells() const;

	protected:
		void load(const std::string& file, std::vector<float>& elements);

		ofBufferObject m_bufferObject;
		ofTexture m_bufferTexture;
		ofVboMesh m_vboMesh;

		ofxRange3f m_coordRange;
		ofxRange1f m_sizeRange;
		ofxRange1f m_densityRange;

		std::size_t m_numCells;
	};
}