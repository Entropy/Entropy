#include "Inflation.h"

#include <entropy/Helpers.h>

namespace entropy
{
	namespace scene
	{
		//--------------------------------------------------------------
		Inflation::Inflation()
			: Base()
		{
			ENTROPY_SCENE_SETUP_LISTENER;
		}

		//--------------------------------------------------------------
		Inflation::~Inflation()
		{

		}
		
		//--------------------------------------------------------------
		void Inflation::setup()
		{
			ENTROPY_SCENE_EXIT_LISTENER;
			ENTROPY_SCENE_RESIZE_LISTENER;
			ENTROPY_SCENE_UPDATE_LISTENER;
			//ENTROPY_SCENE_DRAW_BACK_LISTENER;
			ENTROPY_SCENE_DRAW_WORLD_LISTENER;
			ENTROPY_SCENE_DRAW_FRONT_LISTENER;
			ENTROPY_SCENE_GUI_LISTENER;
			ENTROPY_SCENE_SERIALIZATION_LISTENERS;

			// Marching Cubes
			gpuMarchingCubes.setup();
			
			// Noise Field
			noiseField.setup(gpuMarchingCubes.resolution);

			this->getCamera().setDistance(2);
			this->getCamera().setNearClip(0.01);
			this->getCamera().setFarClip(6.0);

			now = 0;

			// Force resize to set FBOs.
			ofResizeEventArgs args;
			args.width = GetCanvasWidth();
			args.height = GetCanvasHeight();
			this->resize(args);

			// Setup shaders
			shaderBright.load("shaders/vert_full_quad.glsl", "shaders/frag_bright.glsl");

			ofFile frag_blur("shaders/frag_blur.glsl");
			auto frag_blur_src = ofBuffer(frag_blur);
			ofFile vert_blur("shaders/vert_blur.glsl");
			auto vert_blur_src = ofBuffer(vert_blur);

			blurV.setupShaderFromSource(GL_VERTEX_SHADER,
				"#version 330\n#define PASS_V\n#define BLUR9\n" +
				vert_blur_src.getText());
			blurV.setupShaderFromSource(GL_FRAGMENT_SHADER,
				"#version 330\n#define PASS_V\n#define BLUR9\n" +
				frag_blur_src.getText());
			blurV.bindDefaults();
			blurV.linkProgram();

			blurH.setupShaderFromSource(GL_VERTEX_SHADER,
				"#version 330\n#define PASS_H\n#define BLUR9\n" +
				vert_blur_src.getText());
			blurH.setupShaderFromSource(GL_FRAGMENT_SHADER,
				"#version 330\n#define PASS_H\n#define BLUR9\n" +
				frag_blur_src.getText());
			blurH.bindDefaults();
			blurH.linkProgram();

			tonemap.load("shaders/vert_full_quad.glsl", "shaders/frag_tonemap.glsl");

		}

		//--------------------------------------------------------------
		void Inflation::exit()
		{

		}

		//--------------------------------------------------------------
		void Inflation::resize(ofResizeEventArgs & args)
		{
			ofFbo::Settings settings;
			settings.width = args.width;
			settings.height = args.height;
			settings.textureTarget = GL_TEXTURE_2D;
			settings.internalformat = GL_RGBA32F;

			for (int i = 0; i < 2; ++i) {
				fboPost[i].allocate(settings);
			}
		}

		//--------------------------------------------------------------
		void Inflation::update(double & dt)
		{
			if (parameters.runSimulation) {
				now += ofGetElapsedTimef();
				auto inflation = false;
				noiseField.update(inflation);
				if (inflation) {
					//parameters.marchingCubes.scale += ofGetElapsedTimef() * 0.1f;
				}
			}
		}

		float gaussian(float x, float mu, float sigma) {
			auto d = x - mu;
			auto n = 1.0 / (sqrtf(glm::two_pi<float>()) * sigma);
			return exp(-d * d / (2.0 * sigma * sigma)) * n;
		}

		//--------------------------------------------------------------
		void Inflation::drawWorld()
		{
			//ofDisableDepthTest();
			ofEnableDepthTest();
			this->getCamera().setNearClip(0.01);
			this->getCamera().setFarClip(6.0);

			if (parameters.render.debug) {
				noiseField.draw(this->gpuMarchingCubes.isoLevel);
			}
			else {
				if (parameters.render.additiveBlending) {
					ofEnableBlendMode(OF_BLENDMODE_ADD);
				}
				else {
					ofEnableBlendMode(OF_BLENDMODE_ALPHA);
				}
				if (gpuMarchingCubes.shadeNormals) {
					ofEnableDepthTest();
				}

				//ofScale(parameters.marchingCubes.scale);

				gpuMarchingCubes.draw(noiseField.getTexture());

				if (gpuMarchingCubes.shadeNormals) {
					//ofDisableDepthTest();
				}

				ofEnableBlendMode(OF_BLENDMODE_ALPHA);
			}
		}

		//--------------------------------------------------------------
		void Inflation::drawFront()
		{
			ofDrawBitmapString(ofGetFrameRate(), ofGetWidth() - 100, 20);

			ofDrawBitmapString(timeToSetIso, ofGetWidth() - 100, 40);
			ofDrawBitmapString(timeToUpdate, ofGetWidth() - 100, 60);
		}

		//--------------------------------------------------------------
		bool Inflation::postProcess(const ofTexture & srcTexture, const ofFbo & dstFbo) 
		{
			return false;
		}

		//--------------------------------------------------------------
		void Inflation::gui(ofxPreset::Gui::Settings & settings)
		{
			ofxPreset::Gui::SetNextWindow(settings);
			if (ofxPreset::Gui::BeginWindow(this->parameters.getName(), settings))
			{
				ofxPreset::Gui::AddParameter(this->parameters.runSimulation);

				if (ImGui::CollapsingHeader(this->gpuMarchingCubes.parameters.getName().c_str(), nullptr, true, true)) {
					ofxPreset::Gui::AddParameter(this->gpuMarchingCubes.resolution);
					ofxPreset::Gui::AddParameter(this->gpuMarchingCubes.isoLevel);
				}

				if (ImGui::CollapsingHeader(this->parameters.render.getName().c_str(), nullptr, true, true))
				{
					ofxPreset::Gui::AddParameter(this->parameters.render.debug);
					ofxPreset::Gui::AddParameter(this->gpuMarchingCubes.wireframe);
					ofxPreset::Gui::AddParameter(this->gpuMarchingCubes.fill);
					ofxPreset::Gui::AddParameter(this->gpuMarchingCubes.shadeNormals);
					ofxPreset::Gui::AddParameter(this->parameters.render.additiveBlending);

					ofxPreset::Gui::AddParameter(this->gpuMarchingCubes.fogEnabled);
					ofxPreset::Gui::AddParameter(this->gpuMarchingCubes.fogMaxDistance);
					ofxPreset::Gui::AddParameter(this->gpuMarchingCubes.fogMinDistance);
					ofxPreset::Gui::AddParameter(this->gpuMarchingCubes.fogPower);

					ofxPreset::Gui::AddParameter(this->gpuMarchingCubes.fillAlpha);
					ofxPreset::Gui::AddParameter(this->gpuMarchingCubes.wireframeAlpha);

					auto numPoints = 100;
					ImGui::PlotLines("Fog funtion", this->gpuMarchingCubes.getFogFunctionPlot(numPoints).data(), numPoints);
				}

				ofxPreset::Gui::AddGroup(this->noiseField.parameters, settings);
			}
			ofxPreset::Gui::EndWindow(settings);
		}

		//--------------------------------------------------------------
		void Inflation::serialize(nlohmann::json & json)
		{
			ofxPreset::Serializer::Serialize(json, this->noiseField.parameters);
			ofxPreset::Serializer::Serialize(json, this->gpuMarchingCubes.parameters);
		}

		//--------------------------------------------------------------
		void Inflation::deserialize(const nlohmann::json & json)
		{
			ofxPreset::Serializer::Deserialize(json, this->noiseField.parameters);
			ofxPreset::Serializer::Deserialize(json, this->gpuMarchingCubes.parameters);
		}
	}
}
