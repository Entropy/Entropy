#pragma once

#include "ofParameter.h"
#include "ofShader.h"

#include "entropy/Helpers.h"

namespace entropy
{
	namespace surveys
	{
		enum ExtraAttribute
		{
			Mass = 5,
			StarFormationRate = 6
		}; 
		
		class DataSet
		{
		public:
			struct SharedParams
				: ofParameterGroup
			{
				struct : ofParameterGroup
				{
					ofParameter<float> size{ "Size", 8.0f, 0.01f, 32.0f, ofParameterScale::Logarithmic };
					ofParameter<float> attenuation{ "Attenuation", 600.0f, 1.0f, 1000.0f };
					ofParameter<float> fadeNear{ "Fade Near", 100.0f, 0.0f, 1000.0f };
					ofParameter<float> fadeFar{ "Fade Far", 200.0f, 0.0f, 1000.0f };

					PARAM_DECLARE("Points",
						size,
						attenuation,
						fadeNear, fadeFar);
				} point;

				struct : ofParameterGroup
				{
					ofParameter<float> size{ "Size", 8.0f, 0.01f, 64.0f };
					ofParameter<float> attenuation{ "Attenuation", 600.0f, 1.0f, 1000.0f };
					ofParameter<float> alpha{ "Alpha", 1.0f, 0.0f, 1.0f };
					ofParameter<int> density{ "Density", 1000, 100, 100000 };

					PARAM_DECLARE("Shells",
						size,
						attenuation,
						alpha,
						density);
				} shell;

				struct : ofParameterGroup
				{
					ofParameter<bool> useTestModel{ "Use Test Model", false };
					ofParameter<float> geoScale{ "Geo Scale", 8.0f, 0.01f, 32.0f };
					ofParameter<float> alphaScale{ "Alpha Scale", 1.0f, 0.01f, 8.0f, ofParameterScale::Logarithmic };
					ofParameter<float> squashRange{ "Squash Range", 0.2f, 0.0f, 1.0f };
					ofParameter<int> resolution{ "Resolution", 1, 1, 1000 };
					ofParameter<float> clipMass{ "Clip Mass", 0.0f, 0.0f, 1.0f, ofParameterScale::Logarithmic };
					ofParameter<float> maxDensitySize{ "Max Density Size", 10.0f, 1.0f, 1000.0f, ofParameterScale::Logarithmic };
					ofParameter<int> minDensityMod{ "Min Density Mod", 10, 1, 1000, ofParameterScale::Logarithmic };

					PARAM_DECLARE("Model",
						useTestModel,
						geoScale,
						alphaScale,
						squashRange,
						resolution,
						clipMass,
						maxDensitySize, minDensityMod);
				} model;

				PARAM_DECLARE("Shared",
					point,
					shell,
					model);
			};

		public:
			DataSet();
			~DataSet();

			void setup(const std::string & name, const std::string & format, size_t startIdx, size_t endIdx, const std::string & particleType);
			void clear();

			void update(const glm::mat4 & worldTransform, const ofCamera & camera, const ofRectangle & viewport, SharedParams & params, bool updatePicking);
			glm::vec3 getNearestScreenPoint(const glm::vec2 & pt) const;

			void trackAtScreenPoint(const glm::vec2 & pt);

			void drawPoints(ofShader & shader, SharedParams & sharedParams);
			void drawShells(ofShader & shader, SharedParams & sharedParams);
			void drawModels(ofShader & shader, SharedParams & sharedParams, ofVboMesh & mesh);

			struct : ofParameterGroup
			{
				ofParameter<bool> renderPoints{ "Render Points", true };
				ofParameter<bool> renderShells{ "Render Shells", true };
				ofParameter<bool> renderModels{ "Render Models", false };
				ofParameter<float> cutRadius{ "Cut Radius", 0.0f, 0.0f, 1.0f };
				ofParameter<float> minRadius{ "Min Radius", 0.0f, 0.0f, 1.0f };
				ofParameter<float> maxRadius{ "Max Radius", 0.5f, 0.0f, 1.0f };
				ofParameter<float> minLatitude{ "Min Latitude", 0.0f, 0.0f, 1.0f };
				ofParameter<float> maxLatitude{ "Max Latitude", 0.5f, 0.0f, 1.0f };
				ofParameter<float> minLongitude{ "Min Longitude", 0.0f, 0.0f, 1.0f };
				ofParameter<float> maxLongitude{ "Max Longitude", 0.5f, 0.0f, 1.0f };
				ofParameter<ofFloatColor> color{ "Color", ofFloatColor::white };

				PARAM_DECLARE("DataSet", 
					renderPoints, renderShells, renderModels,
					cutRadius,
					minRadius, maxRadius, 
					minLatitude, maxLatitude,
					minLongitude, maxLongitude, 
					color);
			} parameters;

		protected:
			struct InstanceData
			{
				glm::mat4 transform;
				float alpha;
				float starFormationRate;
				uint32_t densityMod;
				float dummy;
			};

			std::size_t loadFragment(const std::string & filePath, const std::string & particleType);
			
			std::vector<glm::vec3> coordinates;
			std::vector<float> masses;
			std::vector<float> starFormationRates;

			std::vector<std::pair<glm::vec2, glm::vec3>> pickingData;

			size_t trackIdx;
			bool findTrackPt;
			glm::vec2 trackScreenPt;

			float minRadius;
			float maxRadius;

			float minMass;
			float maxMass;
			float avgMass;

			float minSfr;
			float maxSfr;

			glm::vec3 mappedRadiusRange;
			glm::vec2 mappedLatitudeRange;
			glm::vec2 mappedLongitudeRange;

			ofVbo vbo;
			ofBufferObject bufferObj;
			int modelCount;

			std::vector<ofEventListener> paramListeners;
		};
	}
}