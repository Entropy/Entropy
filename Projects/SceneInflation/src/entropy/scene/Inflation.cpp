#include "Inflation.h"

#include <entropy/Helpers.h>

namespace entropy
{
	namespace scene
	{
		using namespace glm;
		float smoothstep(float edge0, float edge1, float x){
			float t = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
			return t * t * (3.0 - 2.0 * t);
		}
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
				}
			}));
			for(size_t i = 0; i < targetWavelengths.size(); i++){
				targetAmplitudes[i] = noiseField.octaves[i].amplitude;
			}

			renderer.setup();
			now = 0;
			t_bigbang = 0;

		}
		
		//--------------------------------------------------------------
		void Inflation::setup()
		{
			this->cameras[render::Layout::Back].setDistance(2);
			this->cameras[render::Layout::Back].setNearClip(0.01);
			this->cameras[render::Layout::Back].setFarClip(6.0);

			now = 0;
			t_bigbang = 0;
			state = PreBigBang;
			parameters.Ht = parameters.Ht1;
			parameters.currentFractalFadeScale = parameters.expansionFractalFade;
			parameters.scale = 1;
			resetWavelengths();
		}

		void Inflation::resetWavelengths(){
			auto wl = noiseField.resolution/4;
			for(size_t i = 0; i < targetWavelengths.size(); i++){
				if(i<noiseField.octaves.size()/2){
					targetWavelengths[i] = wl;
					noiseField.octaves[i].wavelength = wl;
					noiseField.octaves[i].amplitude = targetAmplitudes[i];
					noiseField.octaves[i].advanceTime = true;
					noiseField.octaves[i].enabled = true;
					wl /= 2.;
				}else{
					noiseField.octaves[i].enabled = false;
				}
			}
			parameters.hubbleFreq =  8. / noiseField.resolution;
			parameters.Ht = parameters.Ht1;
			parameters.currentFractalFadeScale = parameters.expansionFractalFade;
			parameters.scale = 1;
			noiseField.noiseSpeed = 2;
		}

		float wavelength(size_t idx, float resolution){
			auto wl = resolution/4.;
			if(idx>=4){
				idx-=4;
			}

			for(size_t i=0;i<idx;i++){
				wl/=2;
			}
			return wl;
		}

		void Inflation::updateOctaves(double dt){
			for(size_t i=0; i<noiseField.octaves.size(); i++){
				if(noiseField.octaves[i].enabled && noiseField.octaves[i].frequency > parameters.hubbleFreq){
					//noiseField.octaves[i].amplitude -= sqrt(parameters.Ht * dt);
					noiseField.octaves[i].wavelength += 0.001;//sqrt(parameters.Ht * dt);
					//cout << i << " " << noiseField.octaves[i].wavelength << " " << 1. / hubbleFreq << endl;
				}else if(i<noiseField.octaves.size()/2 && noiseField.octaves[i].advanceTime){
					auto wl = wavelength(i, noiseField.resolution) / parameters.scale;
					cout << "next wl " << wl << endl;
					auto idx = i + noiseField.octaves.size()/2;
					noiseField.octaves[idx].wavelength = wl;
					noiseField.octaves[idx].amplitude = targetAmplitudes[i];
					noiseField.octaves[idx].advanceTime = true;
					noiseField.octaves[idx].enabled = true;
					noiseField.octaves[i].advanceTime = false;
				}else if(i>=noiseField.octaves.size()/2 && noiseField.octaves[i].advanceTime && noiseField.octaves[i].frequency < parameters.hubbleFreq){
					auto wl = wavelength(i, noiseField.resolution) / parameters.scale;
					cout << "next wl " << wl << endl;
					auto idx = i - noiseField.octaves.size()/2;
					noiseField.octaves[idx].wavelength = wl;
					noiseField.octaves[idx].amplitude = targetAmplitudes[idx];
					noiseField.octaves[idx].advanceTime = true;
					noiseField.octaves[i].advanceTime = false;
				}

				if(noiseField.octaves[i].enabled){
					timeSeries[i].push_back(noiseField.octaves[i].wavelength);
					if(timeSeries[i].size()>100){
						timeSeries[i].pop_front();
					}
				}

			}
		}

		//--------------------------------------------------------------
		void Inflation::exit()
		{

		}

		//--------------------------------------------------------------
		void Inflation::update(double dt)
		{
			if (parameters.runSimulation) {
				now += dt;
				switch(state){
					case PreBigBang:
						resetWavelengths();
					break;
					case PreBigBangWobbly:{
						t_from_bigbang = now - t_bigbang;
						float pct = t_from_bigbang / parameters.wobblyDuration;
						pct *= pct;
						for(size_t i=0; i<noiseField.octaves.size(); i++){
							noiseField.octaves[i].wavelength = targetWavelengths[i] * glm::clamp(1 - pct, 0.4f, 1.f);
						}
					}
					break;
					case BigBang:
						t_from_bigbang = now - t_bigbang;
						parameters.scale = t_from_bigbang/parameters.bigBangDuration;
						if(t_from_bigbang > parameters.bigBangDuration){
							state = Expansion;
						}
					break;
					case Expansion:
						t_from_bigbang = now - t_bigbang;
						parameters.scale += dt * parameters.Ht;//t_from_bigbang/parameters.bigBangDuration;
						if(cycle != 0 && parameters.Ht>parameters.Ht3){
							parameters.Ht *= 0.99;
						}
						if(cycle != 0 && noiseField.noiseSpeed > 1){
							noiseField.noiseSpeed * 0.99;
						}
						updateOctaves(dt);
						if(cycle == 0 && parameters.scale>parameters.currentFractalFadeScale){
							t_fade_out = now;
							state = ExpansionFadeOut;
							originalFillAlpha = renderer.fillAlpha;
							originalWireframeAlpha = renderer.wireframeAlpha;
						}
					break;
					case ExpansionFadeOut:
						t_from_bigbang = now - t_bigbang;
						parameters.blobsFade = ofMap(now - t_fade_out, 0, parameters.expansionFractalFadeDuration, 0, 1, true);
						updateOctaves(dt);
						if(ofIsFloatEqual((float)parameters.blobsFade, 1.f)){
							state = ExpansionFadeIn;
							this->cameras[render::Layout::Back].setDistance(1);
							resetWavelengths();
							parameters.scale = 1;
							parameters.Ht = parameters.Ht2;
							t_fade_out = now;
						}
					break;
					case ExpansionFadeIn:
						t_from_bigbang = now - t_bigbang;
						parameters.scale += dt * parameters.Ht;//t_from_bigbang/parameters.bigBangDuration;
						updateOctaves(dt);
						parameters.blobsFade = ofMap(now - t_fade_out, 0, parameters.expansionFractalFadeDuration, 1, 0, true);
						this->cameras[render::Layout::Back].setDistance(ofMap(parameters.blobsFade, 1, 0, 1, 0.5));
						if(ofIsFloatEqual((float)parameters.blobsFade, 0.f)){
							state = Expansion;
							t_bigbang = now;
							renderer.fillAlpha = originalFillAlpha;
							renderer.wireframeAlpha = originalWireframeAlpha;
							renderer.sphericalClip = true;
							cycle += 1;
							if(cycle < 2){
								parameters.currentFractalFadeScale/=2;
							}
						}
					break;
				}

				auto topLeftFront = glm::vec3{-0.5, 0.5, 0.5} * (float)parameters.scale;
				auto bottomRightFront = glm::vec3{-0.5, 0.5, 0.5} * (float)parameters.scale;
				auto projTopLeftFront = this->cameras[render::Layout::Back].worldToScreen(topLeftFront);
				auto projBottomRightFront = this->cameras[render::Layout::Back].worldToScreen(bottomRightFront);
				//cout << projTopLeftFront << endl;

				//this->cameras[render::Layout::Back].setFarClip(6.0 * parameters.scale);
				//noiseField.scale = parameters.scale;
				noiseField.update();
				gpuMarchingCubes.update(noiseField.getTexture());
			}
		}

		//--------------------------------------------------------------
		void Inflation::drawBackWorld()
        {
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
				switch(state){
					case PreBigBang:
					case PreBigBangWobbly:{
						ofEnableBlendMode(OF_BLENDMODE_ALPHA);
						renderer.sphericalClip = true;
						renderer.fadeEdge0 = 0.0;
						renderer.fadeEdge1 = 0.5;
						renderer.draw(gpuMarchingCubes.getGeometry(), 0, gpuMarchingCubes.getNumVertices());
					}break;
					case BigBang:
						ofEnableBlendMode(OF_BLENDMODE_ADD);
						renderer.sphericalClip = true;
						renderer.fadeEdge0 = parameters.scale*parameters.scale;
						renderer.fadeEdge1 = parameters.scale;
						renderer.draw(gpuMarchingCubes.getGeometry(), 0, gpuMarchingCubes.getNumVertices());
					break;
					case Expansion:
						//ofScale(parameters.scale);
						ofEnableBlendMode(OF_BLENDMODE_ADD);
						renderer.sphericalClip = true;
						renderer.draw(gpuMarchingCubes.getGeometry(), 0, gpuMarchingCubes.getNumVertices());
					break;
					case ExpansionFadeOut:
					case ExpansionFadeIn:{
						//ofScale(parameters.scale);
						ofEnableBlendMode(OF_BLENDMODE_ADD);
						auto blobsAlpha = (1-parameters.blobsFade);
						blobsAlpha = sqrt(blobsAlpha);
						renderer.sphericalClip = true;
						renderer.fillAlpha = originalFillAlpha * blobsAlpha;
						renderer.wireframeAlpha = originalWireframeAlpha * blobsAlpha;
						renderer.draw(gpuMarchingCubes.getGeometry(), 0, gpuMarchingCubes.getNumVertices());
					}break;

				}

				if (this->parameters.render.drawBoxInRenderer)
				{
					this->boxes[render::Layout::Back].draw(renderer);
				}

                ofEnableBlendMode(OF_BLENDMODE_ALPHA);
            }
		}

		void Inflation::drawBackOverlay(){
			switch(state){
				case PreBigBangWobbly:{
					float pct = t_from_bigbang / parameters.wobblyDuration;
					if(pct>0.75){
						state = BigBang;
						t_bigbang = now;
					}
				}break;
				case BigBang:{
					auto flashBegin = 0.5;
					auto flashEnd = 0.6;
					float pct = t_from_bigbang / parameters.bigBangDuration;
					if(pct<flashBegin){
						ofSetColor(ofFloatColor(1));
						ofDrawRectangle(0, 0, ofGetViewportWidth(), ofGetViewportHeight());
					}else if(pct<flashEnd){
						ofSetColor(ofFloatColor(1, ofMap(pct, flashBegin, flashEnd, 1, 0)));
						ofDrawRectangle(0, 0, ofGetViewportWidth(), ofGetViewportHeight());
					}
				}break;
				case ExpansionFadeOut:{
					ofEnableBlendMode(OF_BLENDMODE_ADD);
					ofSetColor((ofFloatColor)parameters.fadeOutBgColor * parameters.blobsFade);
					ofDrawRectangle(0, 0, ofGetViewportWidth(), ofGetViewportHeight());
				}
				case ExpansionFadeIn:{
					ofEnableBlendMode(OF_BLENDMODE_ADD);
					ofSetColor((ofFloatColor)parameters.fadeOutBgColor * parameters.blobsFade);
					ofDrawRectangle(0, 0, ofGetViewportWidth(), ofGetViewportHeight());
				}
			}
		}

		//--------------------------------------------------------------
		void Inflation::drawFrontOverlay()
		{
			ofDrawBitmapString(ofGetFrameRate(), ofGetWidth() - 100, 20);

			ofDrawBitmapString(timeToSetIso, ofGetWidth() - 100, 40);
			ofDrawBitmapString(timeToUpdate, ofGetWidth() - 100, 60);

			ofDrawBox(1);
		}

		//--------------------------------------------------------------
		void Inflation::gui(ofxPreset::Gui::Settings & settings)
		{
			ofxPreset::Gui::SetNextWindow(settings);
			if (ofxPreset::Gui::BeginWindow(this->parameters.getName(), settings))
			{
				if(ImGui::Button("Trigger bigbang")){
					if(state == PreBigBang){
						state = PreBigBangWobbly;
						t_bigbang = now;
					}
				}
				ofxPreset::Gui::AddGroup(this->parameters.parameters, settings);

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
					ofxPreset::Gui::AddParameter(this->parameters.render.drawBoxInRenderer);

                    ofxPreset::Gui::AddParameter(this->renderer.wireframe);
                    ofxPreset::Gui::AddParameter(this->renderer.fill);

					ofxPreset::Gui::AddParameter(this->renderer.fogEnabled);
					ofxPreset::Gui::AddParameter(this->renderer.fogStartDistance);
                    ofxPreset::Gui::AddParameter(this->renderer.fogMinDistance);
					ofxPreset::Gui::AddParameter(this->renderer.fogMaxDistance);
                    ofxPreset::Gui::AddParameter(this->renderer.fogPower);
					ofxPreset::Gui::AddParameter(this->renderer.fadeEdge0);
					ofxPreset::Gui::AddParameter(this->renderer.fadeEdge1);
					ofxPreset::Gui::AddParameter(this->renderer.fadePower);
					ofxPreset::Gui::AddParameter(this->renderer.sphericalClip);
					ofxPreset::Gui::AddParameter(this->renderer.useLights);

                    ofxPreset::Gui::AddParameter(this->renderer.fillAlpha);
                    ofxPreset::Gui::AddParameter(this->renderer.wireframeAlpha);

                    int numVertices = this->gpuMarchingCubes.getNumVertices();
                    ImGui::SliderInt("Num Vertices", &numVertices, 0, this->gpuMarchingCubes.getBufferSize()/this->gpuMarchingCubes.getVertexStride());

					auto numPoints = 100;
                    ImGui::PlotLines("Fog Function", this->renderer.getFogFunctionPlot(numPoints).data(), numPoints);
				}

				ofxPreset::Gui::AddGroup(this->noiseField.parameters, settings);
			}
			ofxPreset::Gui::EndWindow(settings);

			ofxPreset::Gui::SetNextWindow(settings);
			if (ofxPreset::Gui::BeginWindow("frequency graphs", settings)){
				for(auto & series: timeSeries){
					if(!series.empty()){
						std::vector<float> values(series.begin(), series.end());
						ImGui::PlotLines("", values.data(), values.size(), 0, 0);
						ImGui::LabelText(std::to_string(values.back()).c_str(),"");
					}
				}
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
			resetWavelengths();
		}
	}
}
