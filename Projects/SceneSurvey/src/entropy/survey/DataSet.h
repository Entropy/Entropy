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
				ofParameter<float> minDistance{ "Min Distance", 0.0f, 0.0f, 1.0f };
				ofParameter<float> maxDistance{ "Max Distance", 0.5f, 0.0f, 1.0f };
				ofParameter<ofFloatColor> color{ "Color", ofFloatColor::white };

				PARAM_DECLARE("DataSet", minDistance, maxDistance, color);
			} parameters;

		protected:
			size_t loadFragment(const string & filePath, std::vector<glm::vec4> & points);

			std::vector<glm::vec4> points;

			ofVbo vbo;
			bool vboDirty;
		};
	}
}