#pragma once

#include "ofMain.h"
#include "ofxPreset.h"

namespace entropy
{
	namespace survey
	{
		class DataSet
		{
		public:
			DataSet();
			~DataSet();

			void setup(const std::string & name, const std::string & format, size_t startIdx, size_t endIdx);
			void clear();

			void update();
			void draw();

			void gui(ofxPreset::Gui::Settings & settings);

			void serialize(nlohmann::json & json);
			void deserialize(const nlohmann::json & json);

			static const size_t MAX_FRAGMENTS = 20;

			struct : ofParameterGroup
			{
				ofParameter<float> minRadius{ "Min Radius", 0.0f, 0.0f, 1.0f };
				ofParameter<float> maxRadius{ "Max Radius", 0.5f, 0.0f, 1.0f };
				ofParameter<float> minLatitude{ "Min Latitude", 0.0f, 0.0f, 1.0f };
				ofParameter<float> maxLatitude{ "Max Latitude", 0.5f, 0.0f, 1.0f };
				ofParameter<float> minLongitude{ "Min Longitude", 0.0f, 0.0f, 1.0f };
				ofParameter<float> maxLongitude{ "Max Longitude", 0.5f, 0.0f, 1.0f };
				ofParameter<ofFloatColor> color{ "Color", ofFloatColor::white };

				PARAM_DECLARE("DataSet", 
					minRadius, maxRadius, 
					minLatitude, maxLatitude,
					minLongitude, maxLongitude, 
					color);
			} parameters;

		protected:
			size_t loadFragment(const string & filePath, std::vector<glm::vec4> & points);

			std::vector<glm::vec4> points;

			ofVbo vbo;
			bool vboDirty;
		};
	}
}