#pragma once

#include "ofxPreset.h"
#include "ofShader.h"

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
			DataSet();
			~DataSet();

			void setup(const std::string & name, const std::string & format, size_t startIdx, size_t endIdx, const std::string & particleType);
			void clear();

			void drawPoints(ofShader & shader, const glm::mat4 & modelTransform);
			void drawModels(ofShader & shader, const glm::mat4 & modelTransform, ofVboMesh & mesh, const ofCamera & camera, float cutoff);

			void gui(ofxImGui::Settings & settings);

			void serialize(nlohmann::json & json);
			void deserialize(const nlohmann::json & json);

			struct : ofParameterGroup
			{
				ofParameter<bool> renderPoints{ "Render Points", true };
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
					renderPoints, renderModels,
					cutRadius,
					minRadius, maxRadius, 
					minLatitude, maxLatitude,
					minLongitude, maxLongitude, 
					color);
			} parameters;

		protected:
			size_t loadFragment(const std::string & filePath, const std::string & particleType);
			
			size_t updateFilteredData(const glm::mat4 & modelTransform, const ofCamera & camera);
			void updateShaderUniforms(ofShader & shader);

			std::vector<glm::vec3> coordinates;
			std::vector<float> masses;
			std::vector<float> starFormationRates;

			float minRadius;
			float maxRadius;

			glm::vec3 mappedRadiusRange;
			glm::vec2 mappedLatitudeRange;
			glm::vec2 mappedLongitudeRange;

			ofVbo vbo;

			ofTexture bufferTex;
			ofBufferObject bufferObj;

			std::vector<ofEventListener> paramListeners;
		};
	}
}