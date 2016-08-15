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


			// Link gui parameters to internal parameters.
			gpuMarchingCubes.resolution.makeReferenceTo(parameters.marchingCubes.resolution);
			gpuMarchingCubes.wireframe.makeReferenceTo(parameters.render.wireframe);
			gpuMarchingCubes.shadeNormals.makeReferenceTo(parameters.render.shadeNormals);
			gpuMarchingCubes.fogEnabled.makeReferenceTo(parameters.render.fogEnabled);
			gpuMarchingCubes.fogMaxDistance.makeReferenceTo(parameters.render.fogMaxDistance);
			gpuMarchingCubes.fogMinDistance.makeReferenceTo(parameters.render.fogMinDistance);
			gpuMarchingCubes.fogPower.makeReferenceTo(parameters.render.fogPower);

			// Marching Cubes
			gpuMarchingCubes.setup();
			
			// Noise Field
			noiseField.setup(gpuMarchingCubes.resolution);

			this->getCamera().setDistance(2);
			//camera.setNearClip(0.1);
			//camera.setFarClip(100000);

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
				noiseField.update(parameters.marchingCubes.inflation);
				if (parameters.marchingCubes.inflation) {
					parameters.marchingCubes.scale += ofGetElapsedTimef() * 0.1f;
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
			ofDisableDepthTest();

			if (parameters.render.debug) {
				noiseField.draw(parameters.marchingCubes.threshold);
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

				ofScale(parameters.marchingCubes.scale);

				gpuMarchingCubes.draw(noiseField.getTexture(), parameters.marchingCubes.threshold);

				if (gpuMarchingCubes.shadeNormals) {
					ofDisableDepthTest();
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

				ofxPreset::Gui::AddGroup(this->parameters.marchingCubes, settings);

				if (ImGui::CollapsingHeader(this->parameters.render.getName().c_str(), nullptr, true, true))
				{
					ofxPreset::Gui::AddParameter(this->parameters.render.debug);
					ofxPreset::Gui::AddParameter(this->parameters.render.drawGrid);
					if (ofxPreset::Gui::AddParameter(this->parameters.render.wireframe))
					{
						gpuMarchingCubes.wireframe = this->parameters.render.wireframe;
					}
					if (ofxPreset::Gui::AddParameter(this->parameters.render.shadeNormals))
					{
						gpuMarchingCubes.shadeNormals = this->parameters.render.shadeNormals;
					}
					ofxPreset::Gui::AddParameter(this->parameters.render.additiveBlending);

					ofxPreset::Gui::AddParameter(this->parameters.render.fogEnabled);
					ofxPreset::Gui::AddParameter(this->parameters.render.fogMaxDistance);
					ofxPreset::Gui::AddParameter(this->parameters.render.fogMinDistance);
					ofxPreset::Gui::AddParameter(this->parameters.render.fogPower);
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
		}

		//--------------------------------------------------------------
		void Inflation::deserialize(const nlohmann::json & json)
		{
			ofxPreset::Serializer::Deserialize(json, this->noiseField.parameters);
		}
	}
}
