#pragma once

#include "ofMain.h"
#include "ofxPreset.h"

namespace entropy
{
	namespace bubbles
	{
		class PoolBase
		{
		public:
			PoolBase();

			virtual void setup() = 0;

			virtual void reset();
			virtual void update(double dt);
			virtual void draw() = 0;

			virtual void gui(ofxPreset::Gui::Settings & settings);

			void setDimensions(int size);
			void setDimensions(const glm::vec2 & dimensions);
			void setDimensions(const glm::vec3 & dimensions);

			const glm::vec3 & getDimensions() const;

			ofParameter<bool> runSimulation{ "Run Simulation", true };

			ofParameter<bool> drawBack{ "Draw Back", true }; 
			ofParameter<bool> drawFront{ "Draw Front", true };

			ofParameter<float> alpha{ "Alpha", 1.0f, 0.0f, 1.0f };

			ofParameter<ofFloatColor> dropColor1{ "Drop Color 1", ofFloatColor(0.29f, 0.56f, 1.0f, 1.0f) };
			ofParameter<ofFloatColor> dropColor2{ "Drop Color 2", ofFloatColor(0.56f, 0.29f, 1.0f, 1.0f) };
			ofParameter<bool> dropping{ "Dropping", true };
			ofParameter<int> dropRate{ "Drop Rate", 1, 1, 60 };

			ofParameter<int> rippleRate{ "Ripple Rate", 1, 1, 60 };

			ofParameter<float> damping{ "Damping", 0.995f, 0.0f, 1.0f };
			ofParameter<float> radius{ "Radius", 30.0f, 1.0f, 60.0f };
			ofParameter<float> ringSize{ "Ring Size", 1.25f, 0.0f, 5.0f };

			ofParameterGroup parameters{ "Pool",
				runSimulation,
				drawFront, drawBack,
				alpha,
				dropColor1, dropColor2,
				dropping, dropRate,
				rippleRate,
				damping, radius, ringSize
			};

			bool resetSimulation;

		protected:
			virtual void addDrop() = 0;
			virtual void stepRipple() = 0;
			virtual void copyResult() = 0;

			glm::vec3 dimensions;

			int currIdx;
			int prevIdx;
			int tempIdx;
		};
	}
}