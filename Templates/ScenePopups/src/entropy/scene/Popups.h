#pragma once

#include "entropy/scene/Base.h"
#include "entropy/popup/Base.h"

namespace entropy
{
	namespace scene
	{
		class PopUps
			: public Base
		{
		public:
			virtual string getName() const override 
			{
				return "entropy::scene::PopUps";
			}

			PopUps();
			~PopUps();

			void setup();
			void exit();
			void resize(ofResizeEventArgs & args);

			void update(double & dt);

			void drawBack();
			void drawWorld();
			void drawFront();

			void gui(ofxPreset::Gui::Settings & settings);

			void serialize(nlohmann::json & json);
			void deserialize(const nlohmann::json & json);

		protected:
			virtual BaseParameters & getParameters() override
			{
				return this->parameters;
			}

			struct : BaseParameters
			{
				
			} parameters;
		};
	}
}