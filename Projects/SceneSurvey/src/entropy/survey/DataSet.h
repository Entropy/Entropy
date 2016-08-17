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
			enum ExtraAttributes
			{
				MASS_ATTRIBUTE = 5,
			};

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

		protected:
			typedef struct
			{
				float longitude;
				float latitude;
				float radius;
				float mass;
				uint32_t fragment;
				ofIndexType index;
			} Point;

			struct : ofParameterGroup
			{
				ofParameter<int> fragments{ "Fragments", 1, 0, MAX_FRAGMENTS };
				ofParameter<float> minDistance{ "Min Distance", 0.0f, 0.0f, 1.0f };
				ofParameter<float> maxDistance{ "Max Distance", 0.5f, 0.0f, 1.0f };
				ofParameter<ofFloatColor> color{ "Color", ofFloatColor::white };

				PARAM_DECLARE("DataSet", fragments, minDistance, maxDistance, color);
			} parameters;

			size_t loadFragment(const string & filePath, std::vector<glm::vec3> & vertices, std::vector<float> & masses);

			std::vector<Point> fragments;
			std::vector<size_t> counts;
			bool enabled[MAX_FRAGMENTS];

			ofVbo vbo;
			bool vboDirty;
		};
	}
}