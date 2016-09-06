#pragma once

#include "entropy/render/Layout.h"
#include "entropy/scene/Base.h"

namespace entropy
{
	namespace scene
	{
		class Interlude
			: public Base
		{
		public:
			string getName() const override
			{
				return "entropy::scene::Interlude";
			}

			Interlude();
			~Interlude();

			void clear() override;

			void setup() override;
			void exit() override;

			void resizeBack(ofResizeEventArgs & args) override;
			void resizeFront(ofResizeEventArgs & args) override;

			void update(double dt) override;

			void drawBackBase() override;
			void drawFrontBase() override;

			void gui(ofxPreset::Gui::Settings & settings) override;

			void deserialize(const nlohmann::json & json) override;

		protected:
			virtual ofParameterGroup & getParameters() override
			{
				return this->parameters;
			}

			struct : ofParameterGroup
			{
				ofParameter<float> dummy{ "Dummy", 0.0f, 0.0f, 1.0f };

				PARAM_DECLARE("Interlude", dummy);
			} parameters;
		};
	}
}
