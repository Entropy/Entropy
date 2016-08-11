#pragma once

#include "ofMain.h"
#include "ofxHDF5.h"
#include "ofxRange.h"
#include "Vapor3DTexture.h"
#include "ofxVolumetrics3D.h"
#include "ofxTexture3d.h"
#include "Constants.h"

namespace ent
{
	enum SnapshotAttributes
	{
		POSITION_ATTRIBUTE = 0,
		SIZE_ATTRIBUTE = 1,
		DENSITY_ATTRIBUTE = 2,
	};
	
	class SnapshotRamses
	{
	public:
		SnapshotRamses();
		~SnapshotRamses();

		struct Settings{
			std::string folder;
			int frameIndex;
			float minDensity;
			float maxDensity;
            size_t worldsize;
            ofxTexture3d volumeTextureFront;
            ofxTexture3d volumeTextureBack;
            #if USE_VOXELS_COMPUTE_SHADER
			    ofShader voxels2texture;
				ofBufferObject voxelsBuffer;
				ofTexture voxelsTexture;
            #endif
            #if USE_PARTICLES_COMPUTE_SHADER
				ofShader particles2texture;
                ofBufferObject particlesBuffer;
                ofTexture particlesTexture;
            #endif
		};

		void setup(Settings & settings);
		void clear();

		void update(ofShader& shader);
		void draw();
		void drawOctree(float minDensity, float maxDensity);

		ofxRange3f& getCoordRange();
		ofxRange1f& getSizeRange();
		ofxRange1f& getDensityRange();

		std::size_t getNumCells() const;
		bool isLoaded() const;
		BoundingBox m_boxRange;

	protected:
		void loadhdf5(const std::string& file, std::vector<float>& elements);
		void precalculate(const std::string folder, int frameIndex, float minDensity, float maxDensity, size_t worldsize);


		ofxRange3f m_coordRange;
		ofxRange1f m_sizeRange;
		ofxRange1f m_densityRange;

		std::size_t m_numCells;
		bool m_bLoaded;
		Vapor3DTexture vaporPixels;
		ofBuffer vaporPixelsBuffer;

		ofVbo m_vboMesh;
		std::string rawFileName;
		std::string particlesFileName;
		std::string metaFileName;
		std::string voxelsFileName;
		std::string particlesGroupsFileName;
	};
}
