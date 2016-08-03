#include "Inflation.h"
#include <entropy/scene/Template.h>

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
			//ENTROPY_SCENE_SERIALIZATION_LISTENERS;

			// Marching Cubes
			gpuMarchingCubes.setup();

			//panelMarchingCubes.loadFromFile("marching-cubes.json");
			//panelMarchingCubes.setPosition(0, 0);


			// Noise Field
			noiseField.setup(gpuMarchingCubes.resolution);

			panelNoiseField.loadFromFile("noise-field.json");
			//panelNoiseField.setPosition(0, panelMarchingCubes.getShape().getMaxY() + 1);
			panelNoiseField.setPosition(0, 0);

			// Render
			//panelRender.loadFromFile("render.json");
			//panelRender.setPosition(0, panelNoiseField.getShape().getMaxY() + 1);
			//record.setSerializable(false);

			camera.setDistance(2);
			camera.setNearClip(0.1);
			camera.setFarClip(100000);

			//guiVisible = true;
			//ofSetBackgroundColor(0);
			now = 0;

			// postpo and recording
			ofFbo::Settings settings;
			settings.width = ofGetWidth();
			settings.height = ofGetHeight();
			settings.textureTarget = GL_TEXTURE_2D;
			settings.internalformat = GL_RGBA32F;

			//fboscene.allocate(settings);
			fbobright.allocate(settings);
			fbo2.allocate(settings);
			settings.internalformat = GL_RGB;
			//finalFbo.allocate(settings);
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
			saverThread.setup(ofGetWidth(), ofGetHeight(), OF_PIXELS_RGB, OF_IMAGE_FORMAT_JPEG);

		}

		//--------------------------------------------------------------
		void Inflation::exit()
		{

		}

		//--------------------------------------------------------------
		void Inflation::resize(ofResizeEventArgs & args)
		{

		}

		//--------------------------------------------------------------
		void Inflation::update(double & dt)
		{
			if (parameters.runSimulation) {
				now += ofGetElapsedTimef();
				noiseField.update(parameters.marchingCubes.inflation);
				if (parameters.marchingCubes.inflation) {
				//noiseField.update(inflation);
				//if (inflation) {
					//scale += ofGetElapsedTimef() * 0.1f;
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
				//camera.begin();
				noiseField.draw(parameters.marchingCubes.threshold);
				//noiseField.draw(threshold);
				//camera.end();
			}
			else {
				if (parameters.render.additiveBlending) {
					ofEnableBlendMode(OF_BLENDMODE_ADD);
				}
				else {
					ofEnableBlendMode(OF_BLENDMODE_ALPHA);
				}
				//fboscene.begin(false);
				if (gpuMarchingCubes.shadeNormals) {
					ofEnableDepthTest();
				}
				//ofClear(0, 255);

				//camera.begin();
				ofScale(parameters.marchingCubes.scale);
				//ofScale(scale);

				gpuMarchingCubes.draw(noiseField.getTexture(), parameters.marchingCubes.threshold);
				//gpuMarchingCubes.draw(noiseField.getTexture(), threshold);

				//camera.end();
				if (gpuMarchingCubes.shadeNormals) {
					ofDisableDepthTest();
				}
				//fboscene.end();

				ofEnableBlendMode(OF_BLENDMODE_ALPHA);
			}

			//if (guiVisible) {
				//panelMarchingCubes.draw();
				//panelNoiseField.draw();
				//panelRender.draw();
			//}
		}

		//--------------------------------------------------------------
		void Inflation::drawFront()
		{
			//if (!bloom) {
			//	fboscene.draw(0, 0);
			//}
			//else {
			//	finalFbo.draw(0, 0);
			//}

			//panelMarchingCubes.draw();
			panelNoiseField.draw();
			//panelRender.draw();
			
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

				fbobright.begin();
				ofClear(0, 255);
				shaderBright.begin();
				blurV.setUniformTexture("tex0", srcTexture, 0);
				shaderBright.setUniform1f("bright_threshold", parameters.render.bloom.brightnessThreshold);
				fullQuad.draw();
				shaderBright.end();
				fbobright.end();

				fbo2.begin();
				ofClear(0, 255);
				blurV.begin();
				blurV.setUniformTexture("tex0", fbobright.getTexture(), 0);
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
				fbo2.end();

				fbobright.begin();
				ofClear(0, 255);
				blurH.begin();
				blurH.setUniformTexture("tex0", fbo2.getTexture(), 0);
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
				fbobright.end();

				//bobright.draw(0,0);

				dstFbo.begin();
				ofClear(0, 255);
				tonemap.begin();
				tonemap.setUniformTexture("tex0", srcTexture, 0);
				tonemap.setUniformTexture("blurred1", fbobright.getTexture(), 1);
				tonemap.setUniform1f("contrast", parameters.render.bloom.contrast);
				tonemap.setUniform1f("brightness", parameters.render.bloom.brightness);
				tonemap.setUniform1f("tonemap_type", parameters.render.bloom.tonemapType);
				fullQuad.draw();
				tonemap.end();
				dstFbo.end();

				if (parameters.record) {
					saverThread.save(dstFbo.getTexture());
				}

				//finalFbo.draw(0, 0);
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

				if (ImGui::CollapsingHeader(this->parameters.marchingCubes.getName().c_str(), nullptr, true, true))
				{
					ofxPreset::Gui::AddParameter(this->parameters.marchingCubes.scale);
					if (ofxPreset::Gui::AddParameter(this->parameters.marchingCubes.resolution))
					{
						this->gpuMarchingCubes.resolution = this->parameters.marchingCubes.resolution;
					}
					ofxPreset::Gui::AddParameter(this->parameters.marchingCubes.threshold);
					ofxPreset::Gui::AddParameter(this->parameters.marchingCubes.inflation);
				}

				if (ImGui::CollapsingHeader(this->parameters.render.getName().c_str(), nullptr, true, true))
				{
					ofxPreset::Gui::AddParameter(this->parameters.render.debug);
					ofxPreset::Gui::AddParameter(this->parameters.render.drawGrid);
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

							ImGui::Columns(2);
							ImGui::RadioButton("Linear", parameters.render.bloom.tonemapType.getRef(), 0); ImGui::NextColumn();
							ImGui::RadioButton("Gamma", parameters.render.bloom.tonemapType.getRef(), 1); ImGui::NextColumn();
							ImGui::RadioButton("Reinhard", parameters.render.bloom.tonemapType.getRef(), 2); ImGui::NextColumn();
							ImGui::RadioButton("Reinhard Alt", parameters.render.bloom.tonemapType.getRef(), 3); ImGui::NextColumn();
							ImGui::RadioButton("Filmic", parameters.render.bloom.tonemapType.getRef(), 4); ImGui::NextColumn();
							ImGui::RadioButton("Uncharted 2", parameters.render.bloom.tonemapType.getRef(), 5); ImGui::NextColumn();
							ImGui::Columns(1);

							ImGui::TreePop();
						}
					}
				}
			}
			ofxPreset::Gui::EndWindow(settings);
		}

		/*
		//--------------------------------------------------------------
		void InflationApp::keyPressed(int key) {
			switch (key) {
			case 'h':
				guiVisible = !guiVisible;
				break;

			case ' ':
				simulationRunning = !simulationRunning;
				break;

			case OF_KEY_TAB:
				ofToggleFullscreen();
				break;

			default:
				break;
			}

		}

		//--------------------------------------------------------------
		void InflationApp::keyReleased(int key) {

		}

		//--------------------------------------------------------------
		void InflationApp::mouseMoved(int x, int y) {

		}

		//--------------------------------------------------------------
		void InflationApp::mouseDragged(int x, int y, int button) {

		}

		//--------------------------------------------------------------
		void InflationApp::mousePressed(int x, int y, int button) {

		}

		//--------------------------------------------------------------
		void InflationApp::mouseReleased(int x, int y, int button) {

		}

		//--------------------------------------------------------------
		void InflationApp::mouseEntered(int x, int y) {

		}

		//--------------------------------------------------------------
		void InflationApp::mouseExited(int x, int y) {

		}

		//--------------------------------------------------------------
		void InflationApp::windowResized(int w, int h) {

		}

		//--------------------------------------------------------------
		void InflationApp::gotMessage(ofMessage msg) {

		}

		//--------------------------------------------------------------
		void InflationApp::dragEvent(ofDragInfo dragInfo) {

		}
		*/
	}
}
