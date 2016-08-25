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
		void Inflation::setup()
		{
			// Marching Cubes
            gpuMarchingCubes.setup(300*1024*1024);
			
			// Noise Field
			noiseField.setup(gpuMarchingCubes.resolution);

            // Renderer
            renderer.setup();

			this->cameras[render::Layout::Back].setDistance(2);
			this->cameras[render::Layout::Back].setNearClip(0.01);
			this->cameras[render::Layout::Back].setFarClip(6.0);

			now = 0;
            ofMesh boxMesh = ofMesh::box(1,1,1,1,1,1);
            boxMesh.getColors().resize(boxMesh.getVertices().size());
            for(auto & c: boxMesh.getColors()){
                c = ofFloatColor(1,0,0,1);
            }
            box.setMesh(boxMesh, GL_STATIC_DRAW);
			renderer.useLights = false;
            //noiseField.update(false);
		}

		//--------------------------------------------------------------
		void Inflation::exit()
		{

		}

		/*
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
		*/

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

		/*
		float gaussian(float x, float mu, float sigma) {
			auto d = x - mu;
			auto n = 1.0 / (sqrtf(glm::two_pi<float>()) * sigma);
			return exp(-d * d / (2.0 * sigma * sigma)) * n;
		}
		*/

		//--------------------------------------------------------------
		void Inflation::drawBackWorld()
        {
			//this->cameras[render::Layout::Back].setNearClip(0.01);
			//this->cameras[render::Layout::Back].setFarClip(6.0);

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

				//ofScale(parameters.marchingCubes.scale);

                //cout << gpuMarchingCubes.getNumVertices() << " vertices at " << ofGetFrameRate() << "fps" << endl;
                renderer.draw(gpuMarchingCubes.getGeometry(), 0, gpuMarchingCubes.getNumVertices());

                //ofSetColor(255);
                //ofNoFill();
                //ofDrawBox(1);
                //renderer.drawElements(box, 0, box.getNumIndices());
				this->boxes[render::Layout::Back].draw(renderer);
				//if (this->boxes[render::Layout::Back].enabled)
				//{
				//	renderer.drawElements(this->boxes[render::Layout::Back].getMesh().getVbo(), 0, this->boxes[render::Layout::Back].getMesh().getNumIndices());
				//}

                ofEnableBlendMode(OF_BLENDMODE_ALPHA);
            }
		}

		//--------------------------------------------------------------
		void Inflation::drawFrontOverlay()
		{
			ofDrawBitmapString(ofGetFrameRate(), ofGetWidth() - 100, 20);

			ofDrawBitmapString(timeToSetIso, ofGetWidth() - 100, 40);
			ofDrawBitmapString(timeToUpdate, ofGetWidth() - 100, 60);
		}

		/*
		//--------------------------------------------------------------
		bool Inflation::postProcess(const ofTexture & srcTexture, const ofFbo & dstFbo) 
		{
			return false;
		}
		*/

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
                    ofxPreset::Gui::AddParameter(this->gpuMarchingCubes.subdivisions);
				}

				if (ImGui::CollapsingHeader(this->parameters.render.getName().c_str(), nullptr, true, true))
				{
                    ofxPreset::Gui::AddParameter(this->parameters.render.debug);
                    ofxPreset::Gui::AddParameter(this->gpuMarchingCubes.shadeNormals);
					ofxPreset::Gui::AddParameter(this->parameters.render.additiveBlending);

                    ofxPreset::Gui::AddParameter(this->renderer.wireframe);
                    ofxPreset::Gui::AddParameter(this->renderer.fill);

					ofxPreset::Gui::AddParameter(this->renderer.fogEnabled);
					ofxPreset::Gui::AddParameter(this->renderer.fogStartDistance);
                    ofxPreset::Gui::AddParameter(this->renderer.fogMinDistance);
					ofxPreset::Gui::AddParameter(this->renderer.fogMaxDistance);
                    ofxPreset::Gui::AddParameter(this->renderer.fogPower);
					ofxPreset::Gui::AddParameter(this->renderer.useLights);

                    ofxPreset::Gui::AddParameter(this->renderer.fillAlpha);
                    ofxPreset::Gui::AddParameter(this->renderer.wireframeAlpha);

                    int numVertices = this->gpuMarchingCubes.getNumVertices();
                    ImGui::SliderInt("num vertices", &numVertices, 0, this->gpuMarchingCubes.getBufferSize()/this->gpuMarchingCubes.getVertexStride());

					auto numPoints = 100;
                    ImGui::PlotLines("Fog funtion", this->renderer.getFogFunctionPlot(numPoints).data(), numPoints);
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
            ofxPreset::Serializer::Serialize(json, this->renderer.parameters);
		}

		//--------------------------------------------------------------
		void Inflation::deserialize(const nlohmann::json & json)
		{
			ofxPreset::Serializer::Deserialize(json, this->noiseField.parameters);
			ofxPreset::Serializer::Deserialize(json, this->gpuMarchingCubes.parameters);
            ofxPreset::Serializer::Deserialize(json, this->renderer.parameters);
		}
	}
}
