#include "Inflation.h"

#include <entropy/Helpers.h>

namespace entropy
{
	namespace scene
	{
		//--------------------------------------------------------------
		Inflation::Inflation()
			: Base()
		{}

		//--------------------------------------------------------------
		Inflation::~Inflation()
		{}

		//--------------------------------------------------------------
		void Inflation::init()
		{
			// Marching Cubes
			gpuMarchingCubes.setup(300 * 1024 * 1024);

			// Noise Field
			noiseField.setup(gpuMarchingCubes.resolution);

			// Custom parameter listeners.
			this->parameterListeners.push_back(this->parameters.render.drawBoxInRenderer.newListener([this](bool & value)
			{
				// Automatically disable default box drawing when using renderer.
				if (value)
				{
					this->boxes[render::Layout::Back].autoDraw = false;
					this->boxes[render::Layout::Front].autoDraw = false;
				}
			}));

			// Setup renderers.
			this->renderers[render::Layout::Back].setup();
			this->renderers[render::Layout::Back].parameters.setName("Renderer Back");
			this->renderers[render::Layout::Front].setup();
			this->renderers[render::Layout::Front].parameters.setName("Renderer Front");
		}
		
		//--------------------------------------------------------------
		void Inflation::setup()
		{
			this->cameras[render::Layout::Back].setDistance(2);
			this->cameras[render::Layout::Back].setNearClip(0.01);
			this->cameras[render::Layout::Back].setFarClip(6.0);

			now = 0;
		}

		//--------------------------------------------------------------
		void Inflation::exit()
		{

		}

		//--------------------------------------------------------------
		void Inflation::update(double dt)
		{
			if (parameters.runSimulation) {
				now += ofGetElapsedTimef();
				auto inflation = false;
                noiseField.update(inflation);
                gpuMarchingCubes.update(noiseField.getTexture());
				if (inflation) {
					//parameters.marchingCubes.scale += ofGetElapsedTimef() * 0.1f;
				}
			}
		}

		//--------------------------------------------------------------
		void Inflation::drawBackWorld()
        {
			if (parameters.render.debug) 
			{
				noiseField.draw(this->gpuMarchingCubes.isoLevel);
			}
			else if (parameters.render.renderBack)
			{
				this->drawScene(render::Layout::Back);
			}
		}

		//--------------------------------------------------------------
		void Inflation::drawFrontWorld()
		{
			if (parameters.render.debug)
			{
				noiseField.draw(this->gpuMarchingCubes.isoLevel);
			}
			else if (parameters.render.renderFront)
			{
				this->drawScene(render::Layout::Front);
			}
		}

		//--------------------------------------------------------------
		void Inflation::drawScene(render::Layout layout)
		{
			if (parameters.render.additiveBlending) 
			{
				ofEnableBlendMode(OF_BLENDMODE_ADD);
			}
			else 
			{
				ofEnableBlendMode(OF_BLENDMODE_ALPHA);
			}

			//ofScale(parameters.marchingCubes.scale);

			//cout << gpuMarchingCubes.getNumVertices() << " vertices at " << ofGetFrameRate() << "fps" << endl;
			renderers[layout].draw(gpuMarchingCubes.getGeometry(), 0, gpuMarchingCubes.getNumVertices());

			//ofSetColor(255);
			//ofNoFill();
			//ofDrawBox(1);
			//renderer.drawElements(box, 0, box.getNumIndices());
			if (this->parameters.render.drawBoxInRenderer)
			{
				this->boxes[layout].draw(renderers[layout]);
			}

			ofEnableBlendMode(OF_BLENDMODE_ALPHA);
		}

		//--------------------------------------------------------------
		void Inflation::drawFrontOverlay()
		{
			ofDrawBitmapString(ofGetFrameRate(), ofGetWidth() - 100, 20);

			ofDrawBitmapString(timeToSetIso, ofGetWidth() - 100, 40);
			ofDrawBitmapString(timeToUpdate, ofGetWidth() - 100, 60);
		}

		//--------------------------------------------------------------
		void Inflation::gui(ofxPreset::Gui::Settings & settings)
		{
			ofxPreset::Gui::SetNextWindow(settings);
			if (ofxPreset::Gui::BeginWindow(this->parameters.getName(), settings))
			{
				ofxPreset::Gui::AddParameter(this->parameters.runSimulation);

				if (ofxPreset::Gui::BeginTree(this->gpuMarchingCubes.parameters, settings))
				{
					ofxPreset::Gui::AddParameter(this->gpuMarchingCubes.resolution);
					ofxPreset::Gui::AddParameter(this->gpuMarchingCubes.isoLevel);
					ofxPreset::Gui::AddParameter(this->gpuMarchingCubes.subdivisions);

					int numVertices = this->gpuMarchingCubes.getNumVertices();
					ImGui::SliderInt("Num Vertices", &numVertices, 0, this->gpuMarchingCubes.getBufferSize() / this->gpuMarchingCubes.getVertexStride());

					ofxPreset::Gui::EndTree(settings);
				}

				if (ofxPreset::Gui::BeginTree(this->parameters.render, settings))
				{
					ofxPreset::Gui::AddParameter(this->parameters.render.debug);
					ofxPreset::Gui::AddParameter(this->gpuMarchingCubes.shadeNormals);
					ofxPreset::Gui::AddParameter(this->parameters.render.additiveBlending);
					ofxPreset::Gui::AddParameter(this->parameters.render.drawBoxInRenderer);

					ofxPreset::Gui::AddParameter(this->parameters.render.renderBack);
					if (this->parameters.render.renderBack)
					{
						if (ofxPreset::Gui::BeginTree(this->renderers[render::Layout::Back].parameters, settings))
						{
							ofxPreset::Gui::AddParameter(this->renderers[render::Layout::Back].wireframe);
							ofxPreset::Gui::AddParameter(this->renderers[render::Layout::Back].fill);

							ofxPreset::Gui::AddParameter(this->renderers[render::Layout::Back].fogEnabled);
							ofxPreset::Gui::AddParameter(this->renderers[render::Layout::Back].fogStartDistance);
							ofxPreset::Gui::AddParameter(this->renderers[render::Layout::Back].fogMinDistance);
							ofxPreset::Gui::AddParameter(this->renderers[render::Layout::Back].fogMaxDistance);
							ofxPreset::Gui::AddParameter(this->renderers[render::Layout::Back].fogPower);
							ofxPreset::Gui::AddParameter(this->renderers[render::Layout::Back].useLights);

							ofxPreset::Gui::AddParameter(this->renderers[render::Layout::Back].fillAlpha);
							ofxPreset::Gui::AddParameter(this->renderers[render::Layout::Back].wireframeAlpha);

							static const auto kNumPoints = 100;
							ImGui::PlotLines("Fog Function", this->renderers[render::Layout::Back].getFogFunctionPlot(kNumPoints).data(), kNumPoints);

							ofxPreset::Gui::EndTree(settings);
						}
					}

					ofxPreset::Gui::AddParameter(this->parameters.render.renderFront);
					if (this->parameters.render.renderFront)
					{
						if (ofxPreset::Gui::BeginTree(this->renderers[render::Layout::Front].parameters, settings))
						{
							ofxPreset::Gui::AddParameter(this->renderers[render::Layout::Front].wireframe);
							ofxPreset::Gui::AddParameter(this->renderers[render::Layout::Front].fill);
																						 
							ofxPreset::Gui::AddParameter(this->renderers[render::Layout::Front].fogEnabled);
							ofxPreset::Gui::AddParameter(this->renderers[render::Layout::Front].fogStartDistance);
							ofxPreset::Gui::AddParameter(this->renderers[render::Layout::Front].fogMinDistance);
							ofxPreset::Gui::AddParameter(this->renderers[render::Layout::Front].fogMaxDistance);
							ofxPreset::Gui::AddParameter(this->renderers[render::Layout::Front].fogPower);
							ofxPreset::Gui::AddParameter(this->renderers[render::Layout::Front].useLights);
																						 
							ofxPreset::Gui::AddParameter(this->renderers[render::Layout::Front].fillAlpha);
							ofxPreset::Gui::AddParameter(this->renderers[render::Layout::Front].wireframeAlpha);

							static const auto kNumPoints = 100;
							ImGui::PlotLines("Fog Function", this->renderers[render::Layout::Front].getFogFunctionPlot(kNumPoints).data(), kNumPoints);

							ofxPreset::Gui::EndTree(settings);
						}
					}

					ofxPreset::Gui::EndTree(settings);
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
			for (auto & it : this->renderers)
			{
				ofxPreset::Serializer::Serialize(json, it.second.parameters);
			}
		}

		//--------------------------------------------------------------
		void Inflation::deserialize(const nlohmann::json & json)
		{
			ofxPreset::Serializer::Deserialize(json, this->noiseField.parameters);
			ofxPreset::Serializer::Deserialize(json, this->gpuMarchingCubes.parameters);
			for (auto & it : this->renderers)
			{
				ofxPreset::Serializer::Deserialize(json, it.second.parameters);
			}
		}
	}
}
