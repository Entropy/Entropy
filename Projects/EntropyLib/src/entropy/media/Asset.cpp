#include "Asset.h"

#include "entropy/Helpers.h"
#include "entropy/util/App.h"

#include "ofxEasing.h"
#include "ofxTimeline.h"

namespace entropy
{
	namespace media
	{
		//--------------------------------------------------------------
		Asset::Asset(Type type)
			: type(type)
			, editing(false)
			, boundsDirty(false)
			, borderDirty(false)
			, wasLoaded(false)
			, twisterFade(-1)
			, twisterSpeed(-1)
			, switchMillis(-1.0f)
			, switchesTrack(nullptr)
			, curvesTrack(nullptr)
			, lfoTrack(nullptr)
			, prevFade(0.0f)
			, lfoVal(0.0f)
			, freePlayMediaLastMs(0.0f)
			, freePlayNeedsInit(false)
		{
			this->parameters.setName(this->getTypeName());
		}

		//--------------------------------------------------------------
		Asset::~Asset()
		{
			this->clear_();
		}

		//--------------------------------------------------------------
		Type Asset::getType() const
		{
			return this->type;
		}

		//--------------------------------------------------------------
		std::string Asset::getTypeName() const
		{
			switch (this->type)
			{
			case Type::Image:
				return "Image";
			case Type::Movie:
				return "Movie";
			case Type::HPV:
				return "HPV";
			case Type::Sound:
				return "Sound";
			default:
				return "Unknown";
			}
		}

		//--------------------------------------------------------------
		bool Asset::renderLayout(render::Layout layout)
		{
			if (layout == render::Layout::Back && this->parameters.render.renderBack)
			{
				return true;
			}
			if (layout == render::Layout::Front && this->parameters.render.renderFront)
			{
				return true;
			}
			return false;
		}

		//--------------------------------------------------------------
		Surface Asset::getSurface()
		{
			return static_cast<Surface>(this->parameters.render.surface.get());
		}

		//--------------------------------------------------------------
		HorzAlign Asset::getHorzAlign()
		{
			return static_cast<HorzAlign>(this->parameters.render.alignHorz.get());
		}

		//--------------------------------------------------------------
		VertAlign Asset::getVertAlign()
		{
			return static_cast<VertAlign>(this->parameters.render.alignVert.get());
		}

		//--------------------------------------------------------------
		SyncMode Asset::getSyncMode()
		{
			return static_cast<SyncMode>(this->parameters.playback.syncMode.get());
		}

		//--------------------------------------------------------------
		float Asset::getTotalFade() const
		{
			return (this->parameters.playback.fadeTrack * this->parameters.playback.fadeTwist * this->parameters.playback.fadeLFO);
		}

		//--------------------------------------------------------------
		std::shared_ptr<Asset> Asset::getLinkedMedia() const
		{
			return this->linkedMedia;
		}

		//--------------------------------------------------------------
		void Asset::setLinkedMedia(std::shared_ptr<Asset> linkedMedia)
		{
			this->linkedMedia = linkedMedia;
		}
		
		//--------------------------------------------------------------
		void Asset::clearLinkedMedia()
		{
			this->linkedMedia.reset();
		}

		//--------------------------------------------------------------
		void Asset::init_(int index, int page, std::shared_ptr<ofxTimeline> timeline)
		{
			this->index = index;
			this->page = page;
			this->timeline = timeline;

			this->addSwitchesTrack();
			if (this->parameters.playback.useFadeTrack)
			{
				this->addCurvesTrack();
			}
			if (this->parameters.playback.useFadeLFO)
			{
				this->addLFOTrack();
			}

			this->parameterListeners.push_back(this->parameters.render.renderBack.newListener([this](bool &)
			{
				this->boundsDirty = true;
			}));
			this->parameterListeners.push_back(this->parameters.render.renderFront.newListener([this](bool &)
			{
				this->boundsDirty = true;
			}));
			this->parameterListeners.push_back(this->parameters.render.size.newListener([this](float &)
			{
				this->boundsDirty = true;
			}));
			this->parameterListeners.push_back(this->parameters.render.anchor.newListener([this](glm::vec2 &)
			{
				this->boundsDirty = true;
			}));
			this->parameterListeners.push_back(this->parameters.render.alignHorz.newListener([this](int &)
			{
				this->boundsDirty = true;
			}));
			this->parameterListeners.push_back(this->parameters.render.alignVert.newListener([this](int &)
			{
				this->boundsDirty = true;
			}));
			this->parameterListeners.push_back(this->parameters.border.width.newListener([this](float &)
			{
				this->borderDirty = true;
			}));

			this->parameterListeners.push_back(this->parameters.playback.useFadeTrack.newListener([this](bool & enabled)
			{
				if (enabled)
				{
					this->addCurvesTrack();
				}
				else
				{
					this->removeCurvesTrack();
				}
			}));
			this->parameterListeners.push_back(this->parameters.playback.fadeTrack.newListener([this](float &)
			{
				const auto syncMode = this->getSyncMode();
				const auto totalFade = this->getTotalFade();
				if (syncMode == SyncMode::FadeControl && this->prevFade == 0.0f && totalFade > 0.0f)
				{
					this->freePlayNeedsInit = true;
				}
				this->prevFade = totalFade;
			}));

			this->parameterListeners.push_back(this->parameters.playback.useFadeTwist.newListener([this](bool & enabled)
			{
				if (enabled)
				{
					this->addTwisterFade();
				}
				else
				{
					this->removeTwisterFade();
				}
			}));
			this->parameterListeners.push_back(this->parameters.playback.fadeKnob.newListener([this](int &)
			{
				if (this->parameters.playback.useFadeTwist)
				{
					this->addTwisterFade();
				}
			}));
			this->parameterListeners.push_back(this->parameters.playback.fadeTwist.newListener([this](float &)
			{
				const auto syncMode = this->getSyncMode();
				const auto totalFade = this->getTotalFade();
				if (syncMode == SyncMode::FadeControl && this->prevFade == 0.0f && totalFade > 0.0f)
				{
					this->freePlayNeedsInit = true;
				}
				this->prevFade = totalFade;
			}));

			this->parameterListeners.push_back(this->parameters.playback.useFadeLFO.newListener([this](bool & enabled)
			{
				if (enabled)
				{
					this->addLFOTrack();
				}
				else
				{
					this->removeLFOTrack();
				}

				this->lfoVal = 0.0f;
			}));
			this->parameterListeners.push_back(this->parameters.playback.fadeLFO.newListener([this](float &)
			{
				const auto syncMode = this->getSyncMode();
				const auto totalFade = this->getTotalFade();
				if (syncMode == SyncMode::FadeControl && this->prevFade == 0.0f && totalFade > 0.0f)
				{
					this->freePlayNeedsInit = true;
				}
				this->prevFade = totalFade;
			}));

			this->parameterListeners.push_back(this->parameters.playback.syncMode.newListener([this](int &)
			{
				const auto syncMode = this->getSyncMode();
				if (syncMode == SyncMode::FreePlay)
				{
					this->freePlayNeedsInit = true;
				}
				else if (syncMode == SyncMode::FadeControl && this->getTotalFade() > 0.0f)
				{
					this->freePlayNeedsInit = true;
				}
			}));

			this->parameterListeners.push_back(this->parameters.playback.useSpeedTwist.newListener([this](bool & enabled)
			{
				if (enabled)
				{
					this->addTwisterSpeed();
				}
				else
				{
					this->removeTwisterSpeed();
				}
			}));
			this->parameterListeners.push_back(this->parameters.playback.speedKnob.newListener([this](int &)
			{
				if (this->parameters.playback.useSpeedTwist)
				{
					this->addTwisterSpeed();
				}
			}));

			this->init();
		}

		//--------------------------------------------------------------
		void Asset::clear_()
		{
			this->clear();

			this->parameterListeners.clear();

			this->removeSwitchesTrack();
			this->removeCurvesTrack();
			this->removeLFOTrack();
			this->timeline.reset();
		}

		//--------------------------------------------------------------
		void Asset::setup_()
		{
			this->setup();

			this->boundsDirty = true;
		}

		//--------------------------------------------------------------
		void Asset::exit_()
		{
			this->exit();
		}

		//--------------------------------------------------------------
		void Asset::resize_(ofResizeEventArgs & args)
		{
			// Update right away so that event listeners can use the new bounds.
			this->updateBounds();
			
			this->resize(args);
		}

		//--------------------------------------------------------------
		void Asset::update_(double dt)
		{
			if (this->boundsDirty)
			{
				this->updateBounds();
			}

			if (this->curvesTrack != nullptr && !this->curvesTrack->getKeyframes().empty())
			{
				this->parameters.playback.fadeTrack = this->curvesTrack->getValue();
			}
			else
			{
				this->parameters.playback.fadeTrack = 1.0f;
			}
			if (this->lfoTrack != nullptr && !this->lfoTrack->getKeyframes().empty())
			{
				float lfoSpeed = this->lfoTrack->getValue();
				if (lfoSpeed == 0.0f)
				{
					this->lfoVal = 0.0f;
				}
				else
				{
					this->lfoVal += dt * lfoSpeed * this->parameters.playback.lfoScale;
				}
				if (this->parameters.playback.remapLFO)
				{
					float waveVal = (this->parameters.playback.flipLFO ? sinf(lfoVal - glm::half_pi<float>()) : cosf(lfoVal));
					this->parameters.playback.fadeLFO = waveVal * 0.5f + 0.5f;
				}
				else
				{
					float waveVal = (this->parameters.playback.flipLFO ? sinf(lfoVal) : cosf(lfoVal));
					this->parameters.playback.fadeLFO = std::max(0.0f, waveVal);
				}
			}
			else
			{
				this->parameters.playback.fadeLFO = 1.0f;
			}

			this->switchMillis = -1.0f;
			this->enabled = this->switchesTrack->isOn();
			if (this->enabled)
			{
				// Update the transition if any switch is currently active.
				const auto switchTime = this->switchesTrack->currentTrackTime();
				auto activeSwitch = this->switchesTrack->getActiveSwitchAtMillis(switchTime);
				if (activeSwitch)
				{
					this->switchMillis = switchTime - activeSwitch->timeRange.min;
				}
			}

			if (this->enabled && this->borderDirty)
			{
				this->updateBorder();
			}

			this->update(dt);
		}

		//--------------------------------------------------------------
		void Asset::draw_()
		{
			ofPushStyle();
			{
				if (this->isLoaded() && this->enabled && this->switchMillis >= 0.0f)
				{
					// Draw the background.
					if (this->parameters.render.background->a > 0)
					{
						ofSetColor(this->parameters.render.background.get());
						ofDrawRectangle(this->dstBounds);
					}

					const float totalFade = this->getTotalFade();

					// Draw the border.
					if (this->parameters.border.width > 0.0f)
					{
						ofSetColor(this->parameters.border.color.get(), 255 * totalFade);
						this->borderMesh.draw();
					}

					// Draw the content.
					ofEnableBlendMode(OF_BLENDMODE_ADD);
					ofSetColor(255 * totalFade);
					this->renderContent();
					ofEnableBlendMode(OF_BLENDMODE_ALPHA);
				}
			}
			ofPopStyle();

			this->draw();
		}

		//--------------------------------------------------------------
		void Asset::gui_(ofxImGui::Settings & settings)
		{
			if (!this->editing) return;

			// Add a GUI window for the parameters.
			ofxImGui::SetNextWindow(settings);
			if (ofxImGui::BeginWindow("Media " + ofToString(this->index) + ": " + parameters.getName(), settings, false, &this->editing))
			{
				if (ofxImGui::BeginTree("File", settings))
				{
					if (ImGui::Button("Load..."))
					{
						auto result = ofSystemLoadDialog("Select a file.", false, GetSharedAssetsPath().string());
						if (result.bSuccess)
						{
							if (this->loadMedia(result.filePath))
							{
								const auto relativePath = ofFilePath::makeRelative(GetSharedAssetsPath(), result.filePath);
								const auto testPath = GetSharedAssetsPath().append(relativePath);
								if (ofFile::doesFileExist(testPath.string()))
								{
									this->parameters.filePath = relativePath;
								}
								else
								{
									this->parameters.filePath = result.filePath;
								}
							}
						}
					}
					ImGui::Text("Filename: %s", this->fileName.c_str());

					ofxImGui::EndTree(settings);
				}

				if (ofxImGui::BeginTree(this->parameters.render, settings))
				{
					ofxImGui::AddParameter(this->parameters.render.background);
					ofxImGui::AddParameter(this->parameters.render.renderBack);
					ImGui::SameLine();
					ofxImGui::AddParameter(this->parameters.render.renderFront);
					static std::vector<std::string> surfaceLabels{ "Base", "Overlay" };
					ofxImGui::AddRadio(this->parameters.render.surface, surfaceLabels, 2);
					ofxImGui::AddParameter(this->parameters.render.size);
					ofxImGui::AddParameter(this->parameters.render.anchor);
					static std::vector<std::string> horzLabels{ "Left", "Center", "Right" };
					ofxImGui::AddRadio(this->parameters.render.alignHorz, horzLabels, 3);
					static std::vector<std::string> vertLabels{ "Top", "Middle", "Bottom" };
					ofxImGui::AddRadio(this->parameters.render.alignVert, vertLabels, 3);

					ofxImGui::EndTree(settings);
				}

				if (ofxImGui::BeginTree(this->parameters.color, settings))
				{
					ofxImGui::AddParameter(this->parameters.color.contrast);
					ofxImGui::AddParameter(this->parameters.color.brightness);

					ofxImGui::EndTree(settings);
				}

				if (ofxImGui::BeginTree(this->parameters.border, settings))
				{
					ofxImGui::AddParameter(this->parameters.border.width);
					ofxImGui::AddParameter(this->parameters.border.color);

					ofxImGui::EndTree(settings);
				}

				if (ofxImGui::BeginTree(this->parameters.playback, settings))
				{
					ofxImGui::AddParameter(this->parameters.playback.useFadeTrack);
					if (this->parameters.playback.useFadeTrack)
					{
						ofxImGui::AddParameter(this->parameters.playback.fadeTrack);
					}
					ofxImGui::AddParameter(this->parameters.playback.useFadeTwist);
					ofxImGui::AddStepper(this->parameters.playback.fadeKnob);
					if (this->parameters.playback.useFadeTwist)
					{
						ofxImGui::AddParameter(this->parameters.playback.fadeTwist);
					}
					ofxImGui::AddParameter(this->parameters.playback.useFadeLFO);
					if (this->parameters.playback.useFadeLFO)
					{
						ofxImGui::AddParameter(this->parameters.playback.flipLFO);
						ImGui::SameLine();
						ofxImGui::AddParameter(this->parameters.playback.remapLFO);
						ofxImGui::AddParameter(this->parameters.playback.lfoScale);
						ofxImGui::AddParameter(this->parameters.playback.fadeLFO);
					}
					ofxImGui::AddParameter(this->parameters.playback.loop);
					static std::vector<std::string> syncLabels{ "Free Play", "Timeline", "Fade Control", "Linked Media" };
					ofxImGui::AddRadio(this->parameters.playback.syncMode, syncLabels, 2);
					const auto syncMode = this->getSyncMode();
					if (syncMode == SyncMode::FreePlay || syncMode == SyncMode::FadeControl)
					{
						ofxImGui::AddParameter(this->parameters.playback.freeSpeed);
						ofxImGui::AddParameter(this->parameters.playback.useSpeedTwist);
						ofxImGui::AddStepper(this->parameters.playback.speedKnob);
					}

					ofxImGui::EndTree(settings);
				}
			}
			ofxImGui::EndWindow(settings);
		}

		//--------------------------------------------------------------
		void Asset::serialize_(nlohmann::json & json)
		{
			json["type"] = static_cast<int>(this->type);
			json["page"] = static_cast<int>(this->page);

			ofxPreset::Serializer::Serialize(json, this->parameters);

			this->serialize(json);
		}

		//--------------------------------------------------------------
		void Asset::deserialize_(const nlohmann::json & json)
		{
			ofxPreset::Serializer::Deserialize(json, this->parameters);

			this->deserialize(json);

			this->boundsDirty = true;

			if (!this->parameters.filePath->empty())
			{
				const auto filePath = this->parameters.filePath.get();
				if (ofFilePath::isAbsolute(filePath))
				{
					this->loadMedia(filePath);
				}
				else
				{
					this->loadMedia(GetSharedAssetsPath().string() + filePath);
				}
			}

			if (this->parameters.playback.useFadeTrack)
			{
				this->addCurvesTrack();
			}
			if (this->parameters.playback.useFadeLFO)
			{
				this->addLFOTrack();
			}
		}

		//--------------------------------------------------------------
		bool Asset::shouldPlay()
		{
			// Not loaded.
			if (!this->isLoaded()) return false;

			// No switch at current time.
			if (this->switchMillis < 0.0f)
			{
				// Reset free play start point.
				this->freePlayNeedsInit = true;

				return false;
			}

			const auto durationMs = this->getDurationMs();

			// No duration.
			if (durationMs <= 0.0f) return false;

			const auto syncMode = this->getSyncMode();
			if (syncMode == SyncMode::Timeline)
			{
				// No loop and switch is passed duration.
				if (!this->parameters.playback.loop && durationMs < this->switchMillis) return false;

				// Timeline is not playing.
				if (!this->timeline->getIsPlaying()) return false;
			}
			else if (syncMode == SyncMode::FreePlay)
			{
				if (!this->initFreePlay())
				{
					// No loop and time is passed duration.
					const auto positionMs = this->getPlaybackTimeMs(false);
					if (!this->parameters.playback.loop && durationMs < positionMs) return false;
				}
			}
			else if (syncMode == SyncMode::FadeControl)
			{
				// Fade value at 0.
				if (this->getTotalFade() == 0.0f) return false;

				if (!this->initFreePlay())
				{
					// No loop and time is passed duration.
					const auto positionMs = this->getPlaybackTimeMs(false);
					if (!this->parameters.playback.loop && durationMs < positionMs) return false;
				}
			}
			else // SyncMode::LinkedMedia
			{
				if (this->linkedMedia == nullptr) return false;

				return this->linkedMedia->shouldPlay();
			}

			return true;
		}

		//--------------------------------------------------------------
		uint64_t Asset::getPlaybackTimeMs(bool wrap)
		{
			const uint64_t durationMs = this->getDurationMs();
			if (durationMs == 0) return 0;

			const auto syncMode = this->getSyncMode();
			if (syncMode == SyncMode::Timeline)
			{
				uint64_t positionMs = this->switchMillis;
				while (positionMs > durationMs)
				{
					positionMs -= durationMs;
				}
				return positionMs;
			}

			if (syncMode == SyncMode::FreePlay || syncMode == SyncMode::FadeControl)
			{
				if (this->initFreePlay())
				{
					return this->freePlayMediaStartMs;
				}

				uint64_t deltaMs = ofGetElapsedTimeMillis() - this->freePlayElapsedLastMs;
				uint64_t positionMs = this->freePlayMediaLastMs + deltaMs * this->parameters.playback.freeSpeed;
				while (wrap && positionMs > durationMs)
				{
					positionMs -= durationMs;
				}
				if (wrap)
				{
					this->freePlayElapsedLastMs += deltaMs;
					this->freePlayMediaLastMs = positionMs;
				}
				return positionMs;
			}

			// SyncMode::LinkedMedia
			if (this->linkedMedia != nullptr)
			{
				return this->linkedMedia->getPlaybackTimeMs();
			}

			return 0;
		}

		//--------------------------------------------------------------
		uint64_t Asset::getPlaybackFrame()
		{
			const auto syncMode = this->getSyncMode();

			if (syncMode == SyncMode::Timeline)
			{
				return (this->getPlaybackTimeMs() / static_cast<float>(this->getDurationMs())) * this->getDurationFrames();
			}

			if (syncMode == SyncMode::FreePlay || syncMode == SyncMode::FadeControl)
			{
				return (this->getPlaybackTimeMs() / 1000.0f * this->getFrameRate());
			}

			//else SyncMode::LinkedMedia
			if (this->linkedMedia != nullptr)
			{
				return this->linkedMedia->getPlaybackFrame();
			}

			return 0;
		}

		//--------------------------------------------------------------
		void Asset::addSwitchesTrack()
		{
			if (!this->timeline)
			{
				ofLogError(__FUNCTION__) << "No timeline set, call init_() first!";
				return;
			}
			
			// Add Page if it doesn't already exist.
			const string pageName = MediaTimelinePageName + "_" + ofToString(this->page);
			if (!this->timeline->hasPage(pageName))
			{
				this->timeline->addPage(pageName);
			}
			auto page = this->timeline->getPage(pageName);

			// Add track.
			const auto switchName = "Media_" + ofToString(this->index) + "_" + this->getTypeName();
			if (!page->getTrack(switchName))
			{
				this->timeline->setCurrentPage(pageName);
				this->switchesTrack = this->timeline->addSwitches(switchName);
			}
		}

		//--------------------------------------------------------------
		void Asset::removeSwitchesTrack()
		{
			if (!this->timeline)
			{
				//ofLogWarning(__FUNCTION__) << "No timeline set.";
				return;
			}

			if (this->switchesTrack)
			{
				this->timeline->removeTrack(this->switchesTrack);
				this->switchesTrack = nullptr;
			}

			// TODO: Figure out why this is not working!
			//auto page = this->timeline->getPage(kTimelinePageName);
			//if (page && page->getTracks().empty())
			//{
			//	cout << "Removing page " << page->getName() << endl;
			//	this->timeline->removePage(page);
			//}
		}

		//--------------------------------------------------------------
		bool Asset::addDefaultSwitch()
		{
			if (!this->timeline)
			{
				//ofLogWarning(__FUNCTION__) << "No timeline set.";
				return false;
			}

			if (!this->switchesTrack)
			{
				//ofLogWarning(__FUNCTION__) << "Switches track for Media " << this->index << " does not exist.";
				return false;
			}

			if (this->switchesTrack->getKeyframes().size())
			{
				//ofLogWarning(__FUNCTION__) << "Switches track for Media " << this->index << " already has a switch.";
				return false;
			}

			auto addedSwitch = this->switchesTrack->addSwitch(this->timeline->getCurrentTimeMillis(), this->getDurationMs());
			return (addedSwitch != nullptr);
		}

		//--------------------------------------------------------------
		void Asset::addCurvesTrack()
		{
			if (!this->timeline)
			{
				ofLogError(__FUNCTION__) << "No timeline set, call init_() first!";
				return;
			}

			// Add Page if it doesn't already exist.
			const string pageName = MediaTimelinePageName + "_" + ofToString(this->page);
			if (!this->timeline->hasPage(pageName))
			{
				this->timeline->addPage(pageName);
			}
			auto page = this->timeline->getPage(pageName);

			// Add track.
			const auto fadeName = "Fade_" + ofToString(this->index) + "_" + this->getTypeName();
			if (!page->getTrack(fadeName))
			{
				this->timeline->setCurrentPage(pageName);
				this->curvesTrack = this->timeline->addCurves(fadeName);
			}
		}

		//--------------------------------------------------------------
		void Asset::removeCurvesTrack()
		{
			if (!this->timeline)
			{
				//ofLogWarning(__FUNCTION__) << "No timeline set.";
				return;
			}

			if (this->curvesTrack)
			{
				this->timeline->removeTrack(this->curvesTrack);
				this->curvesTrack = nullptr;
			}

			// TODO: Figure out why this is not working!
			//auto page = this->timeline->getPage(kTimelinePageName);
			//if (page && page->getTracks().empty())
			//{
			//	cout << "Removing page " << page->getName() << endl;
			//	this->timeline->removePage(page);
			//}
		}

		//--------------------------------------------------------------
		void Asset::addLFOTrack()
		{
			if (!this->timeline)
			{
				ofLogError(__FUNCTION__) << "No timeline set, call init_() first!";
				return;
			}

			// Add Page if it doesn't already exist.
			const string pageName = MediaTimelinePageName + "_" + ofToString(this->page);
			if (!this->timeline->hasPage(pageName))
			{
				this->timeline->addPage(pageName);
			}
			auto page = this->timeline->getPage(pageName);

			// Add track.
			const auto lfoName = "LFO_" + ofToString(this->index) + "_" + this->getTypeName();
			if (!page->getTrack(lfoName))
			{
				this->timeline->setCurrentPage(pageName);
				this->lfoTrack = this->timeline->addCurves(lfoName);
			}
		}

		//--------------------------------------------------------------
		void Asset::removeLFOTrack()
		{
			if (!this->timeline)
			{
				//ofLogWarning(__FUNCTION__) << "No timeline set.";
				return;
			}

			if (this->lfoTrack)
			{
				this->timeline->removeTrack(this->lfoTrack);
				this->lfoTrack = nullptr;
			}

			// TODO: Figure out why this is not working!
			//auto page = this->timeline->getPage(kTimelinePageName);
			//if (page && page->getTracks().empty())
			//{
			//	cout << "Removing page " << page->getName() << endl;
			//	this->timeline->removePage(page);
			//}
		}

		//--------------------------------------------------------------
		void Asset::refreshTwisterSync()
		{
			if (this->parameters.playback.useFadeTwist)
			{
				this->addTwisterFade();
			}
			else
			{
				this->removeTwisterFade();
			}

			if (this->parameters.playback.useSpeedTwist)
			{
				this->addTwisterSpeed();
			}
			else
			{
				this->removeTwisterSpeed();
			}
		}

		//--------------------------------------------------------------
		void Asset::addTwisterFade()
		{
#ifdef OFX_PARAMETER_TWISTER
			if (this->parameters.playback.fadeKnob < 16)
			{
				auto twister = GetApp()->getTwister();
				this->removeTwisterFade();
				this->twisterFade = this->parameters.playback.fadeKnob;
				twister->setParam(this->twisterFade, this->parameters.playback.fadeTwist);
			}
#endif
		}

		//--------------------------------------------------------------
		void Asset::removeTwisterFade()
		{
#ifdef OFX_PARAMETER_TWISTER
			if (this->twisterFade != -1)
			{
				auto twister = GetApp()->getTwister();
				twister->clearParam(this->twisterFade);
				this->twisterFade = -1;
			}
#endif
		}

		//--------------------------------------------------------------
		void Asset::addTwisterSpeed()
		{
#ifdef OFX_PARAMETER_TWISTER
			if (this->parameters.playback.speedKnob < 16)
			{
				auto twister = GetApp()->getTwister();
				this->removeTwisterSpeed();
				this->twisterSpeed = this->parameters.playback.speedKnob;
				twister->setParam(this->twisterSpeed, this->parameters.playback.freeSpeed);
			}
#endif
		}

		//--------------------------------------------------------------
		void Asset::removeTwisterSpeed()
		{
#ifdef OFX_PARAMETER_TWISTER
			if (this->twisterSpeed != -1)
			{
				auto twister = GetApp()->getTwister();
				twister->clearParam(this->twisterSpeed);
				this->twisterSpeed = -1;
			}
#endif
		}

		//--------------------------------------------------------------
		void Asset::updateBounds()
		{
			glm::vec2 canvasSize;
			if (this->parameters.render.renderBack)
			{
				canvasSize = glm::vec2(GetCanvasWidth(render::Layout::Back), GetCanvasHeight(render::Layout::Back));
			}
			else
			{
				canvasSize = glm::vec2(GetCanvasWidth(render::Layout::Front), GetCanvasHeight(render::Layout::Front));
			}
			const auto viewportAnchor = canvasSize * this->parameters.render.anchor.get();
			const auto viewportHeight = canvasSize.y * this->parameters.render.size;
			if (this->isLoaded())
			{
				const auto contentRatio = this->getContentWidth() / this->getContentHeight();
				const auto viewportWidth = viewportHeight * contentRatio;
				this->viewport.setFromCenter(viewportAnchor, viewportWidth, viewportHeight);

				// Calculate the source subsection for Aspect Fill.
				const auto viewportRatio = this->viewport.getAspectRatio();
				if (this->viewport.getAspectRatio() > contentRatio)
				{
					this->roi.width = this->getContentWidth();
					this->roi.height = this->roi.width / viewportRatio;
					this->roi.x = 0.0f;
					this->roi.y = (this->getContentHeight() - this->roi.height) * 0.5f;
				}
				else
				{
					this->roi.height = this->getContentHeight();
					this->roi.width = this->roi.height * viewportRatio;
					this->roi.y = 0.0f;
					this->roi.x = (this->getContentWidth() - this->roi.width) * 0.5f;
				}
			}
			else
			{
				this->viewport.setFromCenter(viewportAnchor, viewportHeight, viewportHeight);
			}

			// Adjust anchor alignment.
			if (this->getHorzAlign() == HorzAlign::Left)
			{
				this->viewport.x += this->viewport.width * 0.5f;
			}
			else if (this->getHorzAlign() == HorzAlign::Right)
			{
				this->viewport.x -= this->viewport.width * 0.5f;
			}
			if (this->getVertAlign() == VertAlign::Top)
			{
				this->viewport.y += this->viewport.height * 0.5f;
			}
			else if (this->getVertAlign() == VertAlign::Bottom)
			{
				this->viewport.y -= this->viewport.height * 0.5f;
			}

			// Set subsection bounds.
			this->dstBounds = this->viewport;
			this->srcBounds = this->roi;

			this->borderDirty = true;

			this->boundsDirty = false;
		}

		//--------------------------------------------------------------
		void Asset::updateBorder()
		{
			this->borderMesh.clear();
			
			float borderWidth = this->parameters.border.width;
			if (borderWidth > 0.0f)
			{
				// Rebuild the border mesh.
				this->borderMesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);

				const auto borderBounds = ofRectangle(dstBounds.x - borderWidth, dstBounds.y - borderWidth, dstBounds.width + 2.0f * borderWidth, dstBounds.height + 2.0f * borderWidth);
				this->borderMesh.addVertex(glm::vec3(borderBounds.getMinX(), borderBounds.getMinY(), 0.0f));
				this->borderMesh.addVertex(glm::vec3(this->dstBounds.getMinX(), this->dstBounds.getMinY(), 0.0f));
				this->borderMesh.addVertex(glm::vec3(borderBounds.getMaxX(), borderBounds.getMinY(), 0.0f));
				this->borderMesh.addVertex(glm::vec3(this->dstBounds.getMaxX(), this->dstBounds.getMinY(), 0.0f));
				this->borderMesh.addVertex(glm::vec3(borderBounds.getMaxX(), borderBounds.getMaxY(), 0.0f));
				this->borderMesh.addVertex(glm::vec3(this->dstBounds.getMaxX(), this->dstBounds.getMaxY(), 0.0f));
				this->borderMesh.addVertex(glm::vec3(borderBounds.getMinX(), borderBounds.getMaxY(), 0.0f));
				this->borderMesh.addVertex(glm::vec3(this->dstBounds.getMinX(), this->dstBounds.getMaxY(), 0.0f));

				this->borderMesh.addIndex(0);
				this->borderMesh.addIndex(1);
				this->borderMesh.addIndex(2);
				this->borderMesh.addIndex(3);
				this->borderMesh.addIndex(4);
				this->borderMesh.addIndex(5);
				this->borderMesh.addIndex(6);
				this->borderMesh.addIndex(7);
				this->borderMesh.addIndex(0);
				this->borderMesh.addIndex(1);
			}

			this->borderDirty = false;
		}
	}
}
