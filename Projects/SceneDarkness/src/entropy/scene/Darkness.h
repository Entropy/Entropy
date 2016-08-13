#pragma once

#include "entropy/scene/Base.h"

namespace entropy
{
	namespace scene
	{
		class Darkness
			: public Base
		{
		public:
			enum ExtraAttributes
			{
				MASS_ATTRIBUTE = 5,
			}; 
			
			string getName() const override
			{
				return "entropy::scene::Darkness";
			}

			Darkness();
			~Darkness();

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
			void loadData(const string & filePath);

			typedef struct
			{
				float longitude;
				float latitude;
				float radius;
			} Coordinate;

			ofShader shader;
			ofVboMesh vboMesh;
			ofTexture texture;

		protected:
			BaseParameters & getParameters() override
			{
				return this->parameters;
			}

			struct : BaseParameters
			{
				ofParameter<float> scale{ "Scale", 1.0f, 0.01f, 20.0f };
				ofParameter<float> pointSize{ "Point Size", 8.0f, 0.1f, 64.0f };
				ofParameter<bool> useSprites{ "Use Sprites", true };

				PARAM_DECLARE("Darkness", scale, pointSize, useSprites);
			} parameters;
		};
	}
}