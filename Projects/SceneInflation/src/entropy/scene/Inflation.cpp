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

			// Link gui parameters to internal parameters.
			marchingCubesEventListeners.push_back(parameters.marchingCubes.resolution.newListener([this](int & resolution)
			{
				gpuMarchingCubes.resolution = resolution;
			}));
			marchingCubesEventListeners.push_back(parameters.render.wireframe.newListener([this](bool & wireframe)
			{
				gpuMarchingCubes.wireframe = wireframe;
			}));
			marchingCubesEventListeners.push_back(parameters.render.shadeNormals.newListener([this](bool & shadeNormals)
			{
				gpuMarchingCubes.shadeNormals = shadeNormals;
			}));
			
			// Noise Field
			noiseField.setup(gpuMarchingCubes.resolution);

			//camera.setDistance(2);
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
			if (parameters.render.bloom.enabled)
			{
				auto texel_size = glm::vec2(1. / float(srcTexture.getWidth()), 1. / float(srcTexture.getHeight()));

				auto w0 = gaussian(0.0, 0.0, parameters.render.bloom.sigma);
				auto w1 = gaussian(1.0, 0.0, parameters.render.bloom.sigma);
				auto w2 = gaussian(2.0, 0.0, parameters.render.bloom.sigma);
				auto w3 = gaussian(3.0, 0.0, parameters.render.bloom.sigma);
				auto w4 = gaussian(4.0, 0.0, parameters.render.bloom.sigma);
				auto w5 = gaussian(5.0, 0.0, parameters.render.bloom.sigma);
				auto w6 = gaussian(6.0, 0.0, parameters.render.bloom.sigma);
				auto w7 = gaussian(7.0, 0.0, parameters.render.bloom.sigma);
				auto w8 = gaussian(8.0, 0.0, parameters.render.bloom.sigma);
				auto wn = w0 + 2.0 * (w1 + w2 + w3 + w4 + w5 + w6 + w7 + w8);

				ofMesh fullQuad;
				fullQuad.addVertices({ { -1, -1, 0 },{ -1,1,0 },{ 1,1,0 },{ 1,-1,0 } });
				fullQuad.addTexCoords({ { 0,1 },{ 0,0 },{ 1,0 },{ 1,1 } });
				fullQuad.setMode(OF_PRIMITIVE_TRIANGLE_FAN);

				// Pass 0: Brightness
				fboPost[0].begin();
				ofClear(0, 255);
				shaderBright.begin();
				blurV.setUniformTexture("tex0", srcTexture, 0);
				shaderBright.setUniform1f("bright_threshold", parameters.render.bloom.brightnessThreshold);
				fullQuad.draw();
				shaderBright.end();
				fboPost[0].end();

				// Pass 1: Blur Vertical
				fboPost[1].begin();
				ofClear(0, 255);
				blurV.begin();
				blurV.setUniformTexture("tex0", fboPost[0].getTexture(), 0);
				blurV.setUniform2f("texel_size", texel_size);
				blurV.setUniform1f("w0", w0 / wn);
				blurV.setUniform1f("w1", w1 / wn);
				blurV.setUniform1f("w2", w2 / wn);
				blurV.setUniform1f("w3", w3 / wn);
				blurV.setUniform1f("w4", w4 / wn);
				blurV.setUniform1f("w5", w5 / wn);
				blurV.setUniform1f("w6", w6 / wn);
				blurV.setUniform1f("w7", w7 / wn);
				blurV.setUniform1f("w8", w8 / wn);
				fullQuad.draw();
				blurV.end();
				fboPost[1].end();

				// Pass 2: Blur Horizontal
				fboPost[0].begin();
				ofClear(0, 255);
				blurH.begin();
				blurH.setUniformTexture("tex0", fboPost[1].getTexture(), 0);
				blurH.setUniform2f("texel_size", texel_size);
				blurH.setUniform1f("w0", w0 / wn);
				blurH.setUniform1f("w1", w1 / wn);
				blurH.setUniform1f("w2", w2 / wn);
				blurH.setUniform1f("w3", w3 / wn);
				blurH.setUniform1f("w4", w4 / wn);
				blurH.setUniform1f("w5", w5 / wn);
				blurH.setUniform1f("w6", w6 / wn);
				blurH.setUniform1f("w7", w7 / wn);
				blurH.setUniform1f("w8", w8 / wn);
				fullQuad.draw();
				blurH.end();
				fboPost[0].end();

				// Pass 3: Tonemap
				dstFbo.begin();
				ofClear(0, 255);
				tonemap.begin();
				tonemap.setUniformTexture("tex0", srcTexture, 0);
				tonemap.setUniformTexture("blurred1", fboPost[0].getTexture(), 1);
				tonemap.setUniform1f("contrast", parameters.render.bloom.contrast);
				tonemap.setUniform1f("brightness", parameters.render.bloom.brightness);
				tonemap.setUniform1f("tonemap_type", parameters.render.bloom.tonemapType);
				fullQuad.draw();
				tonemap.end();
				dstFbo.end();

				return true;
			}

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
					ofxPreset::Gui::AddParameter(this->parameters.render.bloom.enabled);
					if (this->parameters.render.bloom.enabled)
					{
						ImGui::SetNextTreeNodeOpen(true);
						if (ImGui::TreeNode(this->parameters.render.bloom.getName().c_str()))
						{
							ofxPreset::Gui::AddParameter(this->parameters.render.bloom.brightnessThreshold);
							ofxPreset::Gui::AddParameter(this->parameters.render.bloom.sigma);
							ofxPreset::Gui::AddParameter(this->parameters.render.bloom.contrast);
							ofxPreset::Gui::AddParameter(this->parameters.render.bloom.brightness);

							static vector<string> labels = { "Linear", "Gamma", "Reinhard", "Reinhard Alt", "Filmic", "Uncharted 2" };
							ofxPreset::Gui::AddRadio(parameters.render.bloom.tonemapType, labels, 2);

							ImGui::TreePop();
						}
					}
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
