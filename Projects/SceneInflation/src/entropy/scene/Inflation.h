#pragma once

#include "ofMain.h"

#include "entropy/scene/Base.h"
#include "entropy/inflation/NoiseField.h"
#include "entropy/inflation/GPUMarchingCubes.h"
#include "entropy/inflation/Constants.h"

namespace entropy
{
    namespace scene
    {
		class Inflation
			: public Base
		{
		public:
			string getName() const override
			{
				return "entropy::scene::Inflation";
			}

			Inflation();
			~Inflation();

			void setup();
			void exit();
			void resize(ofResizeEventArgs & args);

			void update(double & dt);

			void drawWorld();
			void drawFront();

			bool postProcess(const ofTexture & srcTexture, const ofFbo & dstFbo) override;

			void gui(ofxPreset::Gui::Settings & settings);

			void serialize(nlohmann::json & json);
			void deserialize(const nlohmann::json & json);

			double now;

			inflation::GPUMarchingCubes gpuMarchingCubes;

			// Noise Field
			inflation::NoiseField noiseField;


			// GUI
			//bool guiVisible = true;
			uint64_t timeToSetIso;
			uint64_t timeToUpdate;


			// Post Effects
			ofFbo fboPost[2];

			ofShader shaderBright;
			ofShader blurV;
			ofShader blurH;
			ofShader tonemap;

		protected:
			BaseParameters & getParameters() override
			{
				return this->parameters;
			}

			struct : BaseParameters
			{
				ofParameter<bool> runSimulation{ "Run Simulation", true };
				
				struct : ofParameterGroup
				{
					ofParameter<float> scale{ "Scale", 1.0f, 1.0f, 100.0f };
					ofParameter<int> resolution{ "Resolution", 1, 1, 512 };
					ofParameter<float> threshold{ "Threshold", 0.345f, 0.0f, 1.0f };
					ofParameter<bool> inflation{ "Inflation", false };

					PARAM_DECLARE("Marching Cubes", scale, resolution, threshold, inflation);
				} marchingCubes;

				struct : ofParameterGroup
				{
					ofParameter<bool> debug{ "Debug", false };
					ofParameter<bool> drawGrid{ "Draw Grid", true };
					ofParameter<bool> wireframe{ "Wireframe", true };
					ofParameter<bool> shadeNormals{ "Shade Normals", false };
					ofParameter<bool> additiveBlending{ "Additive Blending", false };
					ofParameter<float> fogMaxDistance{ "Fog max dist.", 1.0, 0.1, 10 };
					ofParameter<float> fogMinDistance{ "Fog min dist.", 0.1f, 0.0f, 5.f };
					ofParameter<bool> fogEnabled{ "Fog enabled", true };
					ofParameter<float> fogPower{ "Fog power", 1.f, 0.00001f, 10.f };

					PARAM_DECLARE("Render", debug, drawGrid, wireframe, shadeNormals, additiveBlending, fogMaxDistance, fogMinDistance, fogEnabled, fogPower);
				} render;

				PARAM_DECLARE("Inflation", runSimulation, marchingCubes, render);
			} parameters;
		};
    }
}
