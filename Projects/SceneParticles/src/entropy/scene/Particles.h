#pragma once

#include "ofMain.h"

#include "entropy/scene/Base.h"
#include "entropy/render/WireframeFillRenderer.h"
#include "entropy/particles/ParticleSystem.h"
#include "entropy/particles/Photons.h"
#include "entropy/particles/Environment.h"

namespace entropy
{
	namespace scene
	{
		class Particles
			: public Base
		{
		public:
			static const float HALF_DIM;

		public:
			string getName() const override
			{
				return "entropy::scene::Particles";
			}

			Particles();
			~Particles();

			void init() override;
			void clear() override;

			void setup() override;
			void exit() override;

			void resizeBack(ofResizeEventArgs & args) override;
			void resizeFront(ofResizeEventArgs & args) override;
			
			void update(double dt) override;
			void timelineBangFired(ofxTLBangEventArgs & args) override;

			void drawBackWorld() override;
			void drawFrontWorld() override;

			void gui(ofxPreset::Gui::Settings & settings) override;

			void serialize(nlohmann::json & json) override;
			void deserialize(const nlohmann::json & json) override;

			void drawSystem(entropy::render::Layout layout);

			bool saveState(const string & path);
			bool loadState(const string & path);

		private:
			void reset();

			nm::ParticleSystem particleSystem;
			nm::Photons photons;
			nm::Environment::Ptr environment;
			bool debug;

			std::map<entropy::render::Layout, entropy::render::WireframeFillRenderer> renderers;

            ofShader shader;
			ofShader::TransformFeedbackSettings shaderSettings;
            ofBufferObject feedbackBuffer;
            ofVbo feedbackVbo;
            GLuint numPrimitives, numPrimitivesQuery;
            std::vector<ofLight> pointLights;

			ofParameterGroup & getParameters() override
			{
				return this->parameters;
			}

			struct : ofParameterGroup
			{
				ofParameter<string> stateFile;

				struct : ofParameterGroup
				{
					ofParameter<bool> colorsPerType{ "Color per Type", true };
					ofParameter<bool> additiveBlending{ "Additive Blend", true };
					ofParameter<bool> drawPhotons{ "Draw Photons", true };
					ofParameter<float> ambientLight{ "Ambient Light", 0.001, 0, 0.02 };
					ofParameter<float> attenuation{ "Attenuation", 0.01, 0.0000001, 0.05 };
					ofParameter<float> lightStrength{ "Light Strength", 1, 0, 1 };
					
					PARAM_DECLARE("Rendering", 
						colorsPerType, 
						additiveBlending, 
						drawPhotons, 
						ambientLight, 
						attenuation);
				} rendering;

				PARAM_DECLARE("Particles",
					stateFile,
					rendering);
			} parameters;
		};
	}
}
