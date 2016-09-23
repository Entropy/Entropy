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

		float smootherstep(float edge0, float edge1, float x){
			// Scale, and clamp x to 0..1 range
			x = clamp((x - edge0)/(edge1 - edge0), 0.0f, 1.0f);
			// Evaluate polynomial
			return x*x*x*(x*(x*6 - 15) + 10);
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
			this->populateMappings(this->noiseField.parameters);

			// Setup renderers.
			this->renderers[render::Layout::Back].setup(1);
			this->renderers[render::Layout::Back].parameters.setName("Renderer Back");
			this->populateMappings(this->renderers[render::Layout::Back].parameters);

			this->renderers[render::Layout::Front].setup(1);
			this->renderers[render::Layout::Front].parameters.setName("Renderer Front");
			this->populateMappings(this->renderers[render::Layout::Front].parameters);

			// Custom parameter listeners.
			this->parameterListeners.push_back(this->parameters.render.boxBackRender.newListener([this](bool & value)
			{
				// Automatically disable default box drawing when using renderer.
				if (value)
				{
					this->boxes[render::Layout::Back].autoDraw = false;
				}
			}));

			// Setup big bang.
			for(size_t i=0;i<postBigBangColors.size();i++){
				postBigBangColors[i] = noiseField.octaves[i].color;
			}

			transitionParticles.setup();
			clearParticlesVel.loadCompute("shaders/compute_clear_color.glsl");
		}

		void Inflation::resizeBack(ofResizeEventArgs & args){
			this->renderers[render::Layout::Back].resize(GetCanvas(render::Layout::Back)->getWidth(), GetCanvas(render::Layout::Back)->getHeight());
		}

		void Inflation::resizeFront(ofResizeEventArgs & args){
			this->renderers[render::Layout::Front].resize(GetCanvas(render::Layout::Front)->getWidth(), GetCanvas(render::Layout::Front)->getHeight());
		}

		//--------------------------------------------------------------
		void Inflation::setup()
		{
			if (this->parameters.controlCamera)
			{
				cameraDistanceBeforeBB = 1;
				this->cameras[render::Layout::Back]->setDistanceToTarget(cameraDistanceBeforeBB);
			}
			this->cameras[render::Layout::Back]->nearClip = 0.01f;
			this->cameras[render::Layout::Back]->farClip = 6.0f;
			
			now = 0;
			t_bigbang = 0;
			state = PreBigBang;
			scale = 1;
			noiseField.scale = scale;
			for(size_t i=0;i<noiseField.octaves.size()/2;i++){
				noiseField.octaves[i].color = preBigbangColors[i];
			}
			resetWavelengths();
			needsParticlesUpdate = true;
		}

		//--------------------------------------------------------------
		void Inflation::resetWavelengths()
		{
			float wl = noiseField.resolution/4;
			targetWavelengths[0] = wl;
			noiseField.octaves[0].wavelength = wl;
			noiseField.octaves[0].advanceTime = true;
			noiseField.octaves[0].frequencyTime = 1/wl;
			wl /= 2;
			targetWavelengths[1] = wl;
			noiseField.octaves[1].wavelength = wl;
			noiseField.octaves[1].advanceTime = true;
			noiseField.octaves[1].frequencyTime = 1/wl;
			wl /= 2;
			targetWavelengths[2] = wl;
			noiseField.octaves[2].wavelength = wl;
			noiseField.octaves[2].advanceTime = true;
			noiseField.octaves[2].frequencyTime = 1/wl;
			wl /= 2;
			targetWavelengths[3] = wl;
			noiseField.octaves[3].wavelength = wl;
			noiseField.octaves[3].advanceTime = true;
			noiseField.octaves[3].frequencyTime = 1/wl;
			for(int i=4;i<noiseField.octaves.size();i++){
				noiseField.octaves[i].enabled = false;
			}
		}

		void Inflation::resetWavelength(size_t octave){
			auto wl = noiseField.resolution/4;
			for(size_t i=0;i<octave;i++){
				wl /= 2;
			}
			noiseField.octaves[octave].wavelength = wl / scale;
			noiseField.octaves[octave].advanceTime = true;
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
						for(size_t i=0;i<noiseField.octaves.size()/2;i++){
							noiseField.octaves[i].color = preBigbangColors[i];
						}
					break;
					case PreBigBangWobble:{
						t_from_bigbang = now - t_bigbang;
						float pct = t_from_bigbang/parameters.preBigBangWobbleDuration;
						pct *= pct;
						for(size_t i=0;i<noiseField.octaves.size()/2;i++){
							noiseField.octaves[i].wavelength = targetWavelengths[i] * ofMap(pct, 0, 1, 1, 0.4);
							noiseField.octaves[i].frequencyTime = 1.f/noiseField.octaves[i].wavelength * (1+pct);
							auto color = preBigbangColors[i];
							noiseField.octaves[i].color = color.lerp(postBigBangColors[i], pct * 0.5);
						}
					}break;

					case BigBang:{
						t_from_bigbang = now - t_bigbang;
						scale += dt * parameters.Ht;// t_from_bigbang/parameters.bigBangDuration;
						auto pct = t_from_bigbang/parameters.bigBangDuration;
						if (this->parameters.controlCamera)
						{
							cameras[render::Layout::Back]->setDistanceToTarget(ofMap(pct, 0, 1, cameraDistanceBeforeBB, 0.5));
						}
						if (t_from_bigbang > parameters.bigBangDuration) {
							//resetWavelengths();
							firstCycle = true;
							state = Expansion;
						}
						for(size_t i=0;i<noiseField.octaves.size()/2;i++){
							auto color = preBigbangColors[i];
							noiseField.octaves[i].color = color.lerp(postBigBangColors[i], 0.5 + pct * 0.5);
							noiseField.octaves[i].frequencyTime = 1.f/noiseField.octaves[i].wavelength * ofMap(pct,0,1,2,1);
						}
						noiseField.octaves.back().wavelength = targetWavelengths.back() * glm::clamp(1 - scale * 2, 0.8f, 1.f);
					}break;

					case Expansion:
						t_from_bigbang = now - t_bigbang;
						scale += dt * parameters.Ht;// t_from_bigbang/parameters.bigBangDuration;
						noiseField.scale = scale;
						if (this->parameters.controlCamera)
						{
							if (cameras[render::Layout::Back]->getDistanceToTarget() > 0.5f) 
							{
								auto d = cameras[render::Layout::Back]->getDistanceToTarget();
								d -= dt * parameters.Ht;
								cameras[render::Layout::Back]->setDistanceToTarget(d);
							}
						}
						if(!firstCycle){
							for(size_t i=0;i<noiseField.octaves.size()/2;i++){
								if(noiseField.octaves[i].advanceTime){
									noiseField.octaves[i].wavelength -= dt * parameters.Ht;
									if(noiseField.octaves[i].wavelength < parameters.hubbleWavelength){
										noiseField.octaves[i].advanceTime = false;
										//resetWavelength(i);
									}
								}
							}
						}
					break;
					case ExpansionTransition:{
						float t_EndIn = t_transition + parameters.bbTransitionIn;
						float t_EndPlateau = t_EndIn + parameters.bbTransitionPlateau;
						float t_EndOut = t_EndPlateau + parameters.bbTransitionOut;

						if(now < t_EndIn){
							auto pct = ofMap(now,t_transition,t_EndIn,0,1);
							pct = sqrt(sqrt(sqrt(pct)));
							if(firstCycle){
								parameters.Ht = ofMap(pct, 0, 1, parameters.HtBB, parameters.HtBB * 2);
							}else{
								parameters.Ht = ofMap(pct, 0, 1, parameters.HtPostBB, parameters.HtBB);
							}
							scale  += dt * parameters.Ht;
						}else if(now < t_EndPlateau){
							auto pct = ofMap(now,t_transition,t_EndOut,0,1);
							pct = sqrt(sqrt(sqrt(pct)));
							parameters.Ht = ofMap(pct, 0, 1, parameters.HtBB, parameters.HtPostBB);
							scale  += dt * parameters.Ht;
						}else if(now < t_EndOut){
							auto pct = ofMap(now,t_transition,t_EndOut,0,1);
							pct = sqrt(sqrt(sqrt(pct)));
							parameters.Ht = ofMap(pct, 0, 1, parameters.HtBB, parameters.HtPostBB);
							scale  += dt * parameters.Ht;
						}
						noiseField.scale = scale;
					}break;

					case ParticlesTransition:{
						float alphaParticles = (now - t_from_particles) / parameters.transitionParticlesDuration;
						alphaParticles = glm::clamp(alphaParticles, 0.f, 1.f);
						transitionParticles.color = ofFloatColor(transitionParticles.color, alphaParticles);

						float alphaBlobs = (now - t_from_particles) / parameters.transitionBlobsOutDuration;
						alphaBlobs = 1.0f - glm::clamp(alphaBlobs, 0.f, 1.f);
						//noiseField.speedFactor = alphaBlobs;
						renderers[entropy::render::Layout::Back].alphaFactor = alphaBlobs;
						renderers[entropy::render::Layout::Front].alphaFactor = alphaBlobs;
						this->transitionParticles.update(transitionParticlesPosition, noiseField.getTexture(), now);
					}break;
				}

				noiseField.update();
				if (renderers[entropy::render::Layout::Back].alphaFactor > 0.001 || renderers[entropy::render::Layout::Front].alphaFactor > 0.001) {
					gpuMarchingCubes.update(noiseField.getTexture());
				}
				if (needsParticlesUpdate) {
					this->transitionParticles.setTotalVertices(this->gpuMarchingCubes.getNumVertices());
					this->transitionParticles.update(transitionParticlesPosition, noiseField.getTexture(), now);
					needsParticlesUpdate = false;
				}

				//auto distance = this->getCamera(render::Layout::Back)->getEasyCam().getDistance();
				//this->getCamera(render::Layout::Back)->getEasyCam().orbitDeg(ofGetElapsedTimef()*10.f,0,distance,glm::vec3(0,0,0));

				//transitionParticles.color = ofFloatColor(transitionParticles.color, 0.0);
			}
		}

		//--------------------------------------------------------------
		void Inflation::timelineBangFired(ofxTLBangEventArgs & args)
		{
			static const string kResetFlag = "reset";
			static const string kBigBangFlag = "bigbang";
			static const string kTransitionFlag = "transition";
			static const string kParticlesFlag = "particles";
			if (args.flag.compare(0, kResetFlag.size(), kResetFlag) == 0)
			{
				triggerReset();
				//this->timeline->stop();
			}
			else if (args.flag.compare(0, kBigBangFlag.size(), kBigBangFlag) == 0)
			{
				triggerBigBang();
				this->timeline->play();
			}
			else if (args.flag.compare(0, kTransitionFlag.size(), kTransitionFlag) == 0)
			{
				triggerTransition();
			}
			else if (args.flag.compare(0, kParticlesFlag.size(), kParticlesFlag) == 0)
			{
				triggerParticles();
				this->timeline->play();
			}
		}

		//--------------------------------------------------------------
		void Inflation::drawBackWorld()
		{
			if (this->parameters.render.boxBackRender)
			{
				auto layout = render::Layout::Back;
				auto & camera = this->getCamera(layout)->getEasyCam();

				bool prevClip = renderers[layout].clip;
				float prevFillAlpha = renderers[layout].fillAlpha;
				renderers[layout].clip = false;
				renderers[layout].fillAlpha = this->boxes[layout].alpha;
				this->boxes[layout].draw(renderers[layout], camera);
				renderers[layout].clip = prevClip;
				renderers[layout].fillAlpha = prevFillAlpha;
			}
			
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
		bool Inflation::triggerReset()
		{
			this->setup();
			for (auto & it : this->cameras)
			{
				it.second->reset(false);
			}
			renderers[entropy::render::Layout::Back].alphaFactor = 1;
			renderers[entropy::render::Layout::Front].alphaFactor = 1;

			return true;
		}

		//--------------------------------------------------------------
		bool Inflation::triggerBigBang()
		{
			if (state == PreBigBang)
			{
				t_bigbang = now;
				t_from_bigbang = 0;
				state = PreBigBangWobble;
				return true;
			}

			return false;
		}

		//--------------------------------------------------------------
		bool Inflation::triggerTransition()
		{
			state = ExpansionTransition;
			t_transition = now;
			octavesResetDuringTransition = false;
			return true;
		}

		bool Inflation::triggerParticles(){
			auto vertices = this->gpuMarchingCubes.getNumVertices();
			auto size = vertices * sizeof(glm::vec4) * 2;
			transitionParticlesPosition.allocate(size, GL_STATIC_DRAW);
			this->gpuMarchingCubes.getGeometry().getVertexBuffer().copyTo(transitionParticlesPosition,0,0,size);
			clearParticlesVel.begin();
			transitionParticlesPosition.bindBase(GL_SHADER_STORAGE_BUFFER, 0);
			clearParticlesVel.dispatchCompute(vertices / 1024 + 1, 1, 1);
			clearParticlesVel.end();

			this->transitionParticles.setTotalVertices(vertices);
			state = ParticlesTransition;
			t_from_particles = now;
			return true;
		}

		//--------------------------------------------------------------
		void Inflation::drawScene(render::Layout layout)
		{
			auto & camera = this->getCamera(layout)->getEasyCam();
			switch (state) {
			case PreBigBang:
			case PreBigBangWobble:
				ofEnableBlendMode(OF_BLENDMODE_ALPHA);
				renderers[layout].clip = true;
				renderers[layout].fadeEdge0 = 0.0;
				renderers[layout].fadeEdge1 = 0.5;
				renderers[layout].draw(gpuMarchingCubes.getGeometry(), 0, gpuMarchingCubes.getNumVertices(), camera);
				break;
			case BigBang:
				ofEnableBlendMode(OF_BLENDMODE_ALPHA);
				renderers[layout].fadeEdge0 = 0.0;
				renderers[layout].fadeEdge1 = 0.5;
				renderers[layout].draw(gpuMarchingCubes.getGeometry(), 0, gpuMarchingCubes.getNumVertices(), camera);

				ofEnableBlendMode(OF_BLENDMODE_ADD);
				renderers[layout].fadeEdge0 = scale*scale;
				renderers[layout].fadeEdge1 = scale;
				renderers[layout].draw(gpuMarchingCubes.getGeometry(), 0, gpuMarchingCubes.getNumVertices(), camera);
				break;
			case Expansion:
			case ExpansionTransition:
				ofEnableBlendMode(OF_BLENDMODE_ADD);
				renderers[layout].clip = false;
				renderers[layout].draw(gpuMarchingCubes.getGeometry(), 0, gpuMarchingCubes.getNumVertices(), camera);
				break;
			case ParticlesTransition:
				renderers[layout].clip = false;
				if (renderers[layout].alphaFactor > 0.001) {
					ofEnableBlendMode(OF_BLENDMODE_ADD);
					renderers[layout].draw(gpuMarchingCubes.getGeometry(), 0, gpuMarchingCubes.getNumVertices(), camera);
				}
				ofEnableBlendMode(OF_BLENDMODE_ALPHA);
				auto alphaBlobs = renderers[layout].alphaFactor;
				renderers[layout].alphaFactor = 1;
				renderers[layout].draw(transitionParticles.getVbo(), 0, transitionParticles.getNumVertices(), camera);
				renderers[layout].alphaFactor = alphaBlobs;
				break;
			}

			ofEnableBlendMode(OF_BLENDMODE_ALPHA);
		}

		void Inflation::drawBackOverlay(){
			ofEnableBlendMode(OF_BLENDMODE_ADD);
			switch(state){
				case PreBigBangWobble:
					if(t_from_bigbang > parameters.preBigBangWobbleDuration){
						t_bigbang = now;
						state = BigBang;
						parameters.Ht = parameters.HtBB;
					}
				break;
				case BigBang:
				case Expansion:{
					float pct = t_from_bigbang / parameters.bigBangDuration;
					float pctIn = parameters.bbFlashStart + parameters.bbFlashIn / parameters.bigBangDuration;
					float pctPlateau = pctIn + parameters.bbFlashPlateau / parameters.bigBangDuration;
					float pctOut = pctPlateau + parameters.bbFlashOut / parameters.bigBangDuration;
					if(pct>parameters.bbFlashStart && pct<pctIn){
						auto gray = ofMap(pct, parameters.bbFlashStart, pctIn, 0, 1, true);
						gray *= gray;
						ofSetColor(ofFloatColor(gray,gray));
						ofDrawRectangle(0, 0, ofGetViewportWidth(), ofGetViewportHeight());
					}else if(pct>pctIn && pct<pctPlateau){
						ofSetColor(ofFloatColor(1));
						ofDrawRectangle(0, 0, ofGetViewportWidth(), ofGetViewportHeight());
					}else if(pct>pctPlateau && pct<pctOut){
						auto gray = ofMap(pct, pctPlateau, pctOut, 1, 0, true);
						gray = sqrt(gray);
						ofSetColor(ofFloatColor(gray,gray));
						ofDrawRectangle(0, 0, ofGetViewportWidth(), ofGetViewportHeight());
					}

					if(firstCycle && scale > parameters.bbTransitionFlash){
						state = ExpansionTransition;
						t_transition = now;
						octavesResetDuringTransition = false;
					}

				}break;
				case ExpansionTransition:{
					float t_EndIn = t_transition + parameters.bbTransitionIn;
					float t_EndPlateau = t_EndIn + parameters.bbTransitionPlateau;
					float t_EndOut = t_EndPlateau + parameters.bbTransitionOut;
					if(now<t_EndIn){
						auto gray = ofMap(now, t_transition, t_EndIn, 0, 1, true);
						gray *= gray;
						ofSetColor(ofFloatColor(ofFloatColor(parameters.bbTransitionColor)*gray, gray));
						ofDrawRectangle(0, 0, ofGetViewportWidth(), ofGetViewportHeight());
					}else if(now<t_EndPlateau){
						if(!octavesResetDuringTransition){
							resetWavelengths();
							octavesResetDuringTransition = true;
							scale = 1;
							t_from_bigbang = now;
							//cameras[render::Layout::Back].setDistance(1);
						}
						ofSetColor(ofFloatColor(parameters.bbTransitionColor));
						ofDrawRectangle(0, 0, ofGetViewportWidth(), ofGetViewportHeight());
					}else if(now<t_EndOut){
						auto gray = ofMap(now, t_EndPlateau, t_EndOut, 1, 0, true);
						gray = sqrt(gray);
						ofSetColor(ofFloatColor(ofFloatColor(parameters.bbTransitionColor)*gray, gray));
						ofDrawRectangle(0, 0, ofGetViewportWidth(), ofGetViewportHeight());
					}else{
						state = Expansion;
						firstCycle = false;
					}

				}
			}
		}

		//--------------------------------------------------------------
		void Inflation::gui(ofxPreset::Gui::Settings & settings)
		{
			ofxPreset::Gui::SetNextWindow(settings);
			if (ofxPreset::Gui::BeginWindow(this->parameters.getName(), settings))
			{
				ofxPreset::Gui::AddParameter(this->parameters.runSimulation);
				ofxPreset::Gui::AddParameter(this->parameters.controlCamera);

				if (ImGui::Button("Trigger Reset")) {
					this->triggerReset();
				}
				if (ImGui::Button("Trigger Big Bang")) {
					this->triggerBigBang();
				}
				if (ImGui::Button("Trigger Transition")) {
					this->triggerTransition();
				}
				if (ImGui::Button("Trigger Particles")) {
					this->triggerParticles();
				}

				if (ofxPreset::Gui::BeginTree("Big Bang", settings))
				{
					ofxPreset::Gui::AddParameter(this->parameters.bigBangDuration);
					ofxPreset::Gui::AddParameter(this->parameters.preBigBangWobbleDuration);
					ofxPreset::Gui::AddParameter(this->parameters.Ht);
					ofxPreset::Gui::AddParameter(this->parameters.HtBB);
					ofxPreset::Gui::AddParameter(this->parameters.HtPostBB);
					ofxPreset::Gui::AddParameter(this->parameters.hubbleWavelength);
					ofxPreset::Gui::AddParameter(this->parameters.bbFlashStart);
					ofxPreset::Gui::AddParameter(this->parameters.bbFlashIn);
					ofxPreset::Gui::AddParameter(this->parameters.bbFlashPlateau);
					ofxPreset::Gui::AddParameter(this->parameters.bbFlashOut);

					ofxPreset::Gui::EndTree(settings);
				}

				if (ofxPreset::Gui::BeginTree("Transition", settings))
				{
					ofxPreset::Gui::AddParameter(this->parameters.bbTransitionIn);
					ofxPreset::Gui::AddParameter(this->parameters.bbTransitionOut);
					ofxPreset::Gui::AddParameter(this->parameters.bbTransitionPlateau);
					ofxPreset::Gui::AddParameter(this->parameters.bbTransitionColor);
					ofxPreset::Gui::AddParameter(this->parameters.bbTransitionFlash);

					ofxPreset::Gui::EndTree(settings);
				}

				if (ofxPreset::Gui::BeginTree("Particles", settings))
				{
					ofxPreset::Gui::AddParameter(this->parameters.transitionParticlesDuration);
					ofxPreset::Gui::AddParameter(this->parameters.transitionBlobsOutDuration);
				
					ofxPreset::Gui::EndTree(settings);
				}

				if (ofxPreset::Gui::BeginTree(this->gpuMarchingCubes.parameters, settings))
				{
					ofxPreset::Gui::AddParameter(this->gpuMarchingCubes.resolution);
					ofxPreset::Gui::AddParameter(this->gpuMarchingCubes.isoLevel);
					ofxPreset::Gui::AddParameter(this->gpuMarchingCubes.subdivisions);

					int numVertices = this->gpuMarchingCubes.getNumVertices();
					ImGui::SliderInt("Num Vertices", &numVertices, 0, this->gpuMarchingCubes.getBufferSize() / this->gpuMarchingCubes.getVertexStride());
					numVertices = this->transitionParticles.getNumVertices();
					ImGui::SliderInt("Num Vertices Particles", &numVertices, 0, this->transitionParticles.getNumVertices());

					ofxPreset::Gui::EndTree(settings);
				}

				if (ofxPreset::Gui::BeginTree(this->parameters.render, settings))
				{
					ofxPreset::Gui::AddParameter(this->parameters.render.debug);
					ofxPreset::Gui::AddParameter(this->gpuMarchingCubes.shadeNormals);
					ofxPreset::Gui::AddParameter(this->parameters.render.boxBackRender);

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
							ofxPreset::Gui::AddParameter(this->renderers[render::Layout::Back].fadeEdge0);
							ofxPreset::Gui::AddParameter(this->renderers[render::Layout::Back].fadeEdge1);
							ofxPreset::Gui::AddParameter(this->renderers[render::Layout::Back].fadePower);
							ofxPreset::Gui::AddParameter(this->renderers[render::Layout::Back].sphericalClip);
							ofxPreset::Gui::AddParameter(this->renderers[render::Layout::Back].wobblyClip);
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
							ofxPreset::Gui::AddParameter(this->renderers[render::Layout::Front].fadeEdge0);
							ofxPreset::Gui::AddParameter(this->renderers[render::Layout::Front].fadeEdge1);
							ofxPreset::Gui::AddParameter(this->renderers[render::Layout::Front].fadePower);
							ofxPreset::Gui::AddParameter(this->renderers[render::Layout::Front].sphericalClip);
							ofxPreset::Gui::AddParameter(this->renderers[render::Layout::Front].wobblyClip);
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
				ofxPreset::Gui::AddGroup(this->transitionParticles.parameters, settings);
			}
			ofxPreset::Gui::EndWindow(settings);
		}

		//--------------------------------------------------------------
		void Inflation::serialize(nlohmann::json & json)
		{
			ofxPreset::Serializer::Serialize(json, this->noiseField.parameters);
			ofxPreset::Serializer::Serialize(json, this->gpuMarchingCubes.parameters);
			ofxPreset::Serializer::Serialize(json, this->transitionParticles.parameters);

			// Save Renderer settings.
			auto & jsonRenderers = json["Renderers"];
			for (auto & it : this->renderers)
			{
				ofxPreset::Serializer::Serialize(jsonRenderers, it.second.parameters);
			}
		}

		//--------------------------------------------------------------
		void Inflation::deserialize(const nlohmann::json & json)
		{
			ofxPreset::Serializer::Deserialize(json, this->noiseField.parameters);
			ofxPreset::Serializer::Deserialize(json, this->gpuMarchingCubes.parameters);
			ofxPreset::Serializer::Deserialize(json, this->transitionParticles.parameters);

			// Restore Renderer settings.
			if (json.count("Renderers"))
			{
				auto & jsonRenderers = json["Renderers"];
				for (auto & it : this->renderers)
				{
					ofxPreset::Serializer::Deserialize(jsonRenderers, it.second.parameters);
				}
			}

			resetWavelengths();
		}
	}
}
