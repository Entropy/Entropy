#pragma once

#include "ofxPreset.h"

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

			void draw(ofShader & shader);

			void gui(ofxPreset::Gui::Settings & settings);

			void serialize(nlohmann::json & json);
			void deserialize(const nlohmann::json & json);

			struct : ofParameterGroup
			{
				ofParameter<float> cutRadius{ "Cut Radius", 0.0f, 0.0f, 1.0f };
				ofParameter<float> minRadius{ "Min Radius", 0.0f, 0.0f, 1.0f };
				ofParameter<float> maxRadius{ "Max Radius", 0.5f, 0.0f, 1.0f };
				ofParameter<float> minLatitude{ "Min Latitude", 0.0f, 0.0f, 1.0f };
				ofParameter<float> maxLatitude{ "Max Latitude", 0.5f, 0.0f, 1.0f };
				ofParameter<float> minLongitude{ "Min Longitude", 0.0f, 0.0f, 1.0f };
				ofParameter<float> maxLongitude{ "Max Longitude", 0.5f, 0.0f, 1.0f };
				ofParameter<ofFloatColor> color{ "Color", ofFloatColor::white };

				PARAM_DECLARE("DataSet", 
					cutRadius,
					minRadius, maxRadius, 
					minLatitude, maxLatitude,
					minLongitude, maxLongitude, 
					color);
			} parameters;

		protected:
			size_t loadFragment(const std::string & filePath, const std::string & particleType);

			std::vector<glm::vec3> coordinates;
			std::vector<float> masses;
			std::vector<float> starFormationRates;

			float minRadius;
			float maxRadius;

			ofVbo vbo;
		};
	}
}