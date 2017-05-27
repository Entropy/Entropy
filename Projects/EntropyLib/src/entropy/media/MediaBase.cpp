#include "Base.h"

#include "entropy/Helpers.h"
#include "entropy/util/App.h"

#include "ofxEasing.h"
#include "ofxTimeline.h"

namespace entropy
{
	namespace media
	{
		//--------------------------------------------------------------
		Base::Base(Type type)
			: type(type)
			, editing(false)
			, boundsDirty(false)
			, borderDirty(false)
			, wasLoaded(false)
			, transitionPct(0.0f)
			, switchMillis(-1.0f)
			, switchesTrack(nullptr)
			, prevFade(0.0f)
			, freePlayNeedsInit(false)
		{
			this->parameters.setName(this->getTypeName());
		}

		//--------------------------------------------------------------
		Base::~Base()
		{
			this->clear_();
		}

		//--------------------------------------------------------------
		Type Base::getType() const
		{
			return this->type;
		}

		//--------------------------------------------------------------
		std::string Base::getTypeName() const
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
		bool Base::renderLayout(render::Layout layout)
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
		Surface Base::getSurface()
		{
			return static_cast<Surface>(this->parameters.render.surface.get());
		}

		//--------------------------------------------------------------
		HorzAlign Base::getHorzAlign()
		{
			return static_cast<HorzAlign>(this->parameters.render.alignHorz.get());
		}

		//--------------------------------------------------------------
		VertAlign Base::getVertAlign()
		{
			return static_cast<VertAlign>(this->parameters.render.alignVert.get());
		}

		//--------------------------------------------------------------
		SyncMode Base::getSyncMode()
		{
			return static_cast<SyncMode>(this->parameters.playback.syncMode.get());
		}

		//--------------------------------------------------------------
		std::shared_ptr<Base> Base::getLinkedMedia() const
		{
			return this->linkedMedia;
		}

		//--------------------------------------------------------------
		void Base::setLinkedMedia(std::shared_ptr<Base> linkedMedia)
		{
			this->linkedMedia = linkedMedia;
		}
		
		//--------------------------------------------------------------
		void Base::clearLinkedMedia()
		{
			this->linkedMedia.reset();
		}

		//--------------------------------------------------------------
		void Base::init_(int index, std::shared_ptr<ofxTimeline> timeline)
		{
			this->index = index;
			this->timeline = timeline;

			this->addTimelineTrack();

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

			this->parameterListeners.push_back(this->parameters.playback.syncMode.newListener([this](int &)
			{
				const auto syncMode = this->getSyncMode();
				if (syncMode == SyncMode::FreePlay)
				{
					this->freePlayNeedsInit = true;
				}
				else if (syncMode == SyncMode::FadeControl && this->parameters.playback.fade > 0.0f)
				{
					this->freePlayNeedsInit = true;
				}
			}));
			this->parameterListeners.push_back(this->parameters.playback.fade.newListener([this](float & val)
			{
				const auto syncMode = this->getSyncMode();
				if (syncMode == SyncMode::FadeControl && this->prevFade == 0.0f && val > 0.0f)
				{
					this->freePlayNeedsInit = true;
				}
				this->prevFade = val;
			}));

			this->init();
		}

		//--------------------------------------------------------------
		void Base::clear_()
		{
			this->clear();

			this->parameterListeners.clear();

			this->removeTimelineTrack();
			this->timeline.reset();
		}

		//--------------------------------------------------------------
		void Base::setup_()
		{
			this->setup();

			this->boundsDirty = true;
		}

		//--------------------------------------------------------------
		void Base::exit_()
		{
			this->exit();
		}

		//--------------------------------------------------------------
		void Base::resize_(ofResizeEventArgs & args)
		{
			// Update right away so that event listeners can use the new bounds.
			this->updateBounds();
			
			this->resize(args);
		}

		//--------------------------------------------------------------
		void Base::update_(double dt)
		{
			if (this->boundsDirty)
			{
				this->updateBounds();
			}

			this->switchMillis = -1.0f;
			this->enabled = this->switchesTrack->isOn();
			if (this->enabled)
			{
				// Update the transition if any switch is currently active.
				auto trackTime = this->switchesTrack->currentTrackTime();
				auto activeSwitch = this->switchesTrack->getActiveSwitchAtMillis(trackTime);
				if (activeSwitch)
				{
					this->switchMillis = trackTime - activeSwitch->timeRange.min;

					auto kEasingFunction = ofxeasing::quad::easeIn;
					long transitionDuration = this->parameters.transition.duration * 1000; 
					if (trackTime - activeSwitch->timeRange.min < transitionDuration)
					{
						// Transitioning in.
						this->transitionPct = ofxeasing::map_clamp(trackTime, activeSwitch->timeRange.min, activeSwitch->timeRange.min + transitionDuration, 0.0f, 1.0f ,kEasingFunction);
						this->borderDirty = true;
					}
					else if (activeSwitch->timeRange.max - trackTime < transitionDuration)
					{
						// Transitioning out.
						this->transitionPct = ofxeasing::map_clamp(trackTime, activeSwitch->timeRange.max - transitionDuration, activeSwitch->timeRange.max, 1.0f, 0.0f, kEasingFunction);
						this->borderDirty = true;
					}
					else if (this->transitionPct != 1.0f)
					{
						this->transitionPct = 1.0f;
						this->borderDirty = true;
					}
				}
			}

			if (this->enabled)
			{
				auto transition = static_cast<Transition>(this->parameters.transition.type.get());
				
				// Adjust fade.
				if (transition == Transition::Mix)
				{
					this->parameters.playback.fade = this->transitionPct;
				}
				else if (transition == Transition::Strobe)
				{
					this->parameters.playback.fade = (ofRandomuf() < this->transitionPct) ? 1.0f : 0.0f;
				}

				// Set subsection bounds.
				this->dstBounds = this->viewport;
				this->srcBounds = this->roi;
				if (this->enabled && transition == Transition::Wipe && this->transitionPct < 1.0f)
				{
					this->dstBounds.height = this->transitionPct * this->viewport.height;
					this->dstBounds.y = this->viewport.y + (1.0f - this->transitionPct) * this->viewport.height * 0.5f;

					this->srcBounds.height = this->transitionPct * this->roi.height;
					this->srcBounds.y = this->roi.y + (1.0f - this->transitionPct) * this->roi.height * 0.5f;
				}

				if (this->borderDirty)
				{
					this->updateBorder();
				}
			}

			this->update(dt);
		}

		//--------------------------------------------------------------
		void Base::draw_()
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

					// Draw the border.
					if (this->parameters.border.width > 0.0f)
					{
						ofSetColor(this->parameters.border.color.get(), 255 * this->parameters.playback.fade);
						this->borderMesh.draw();
					}

					// Draw the content.
					ofEnableBlendMode(OF_BLENDMODE_ADD);
					//ofSetColor(255, this->frontAlpha * 255);
					ofSetColor(255 * this->parameters.playback.fade);
					this->renderContent();
					ofEnableBlendMode(OF_BLENDMODE_ALPHA);
				}
			}
			ofPopStyle();

			this->draw();
		}

		//--------------------------------------------------------------
		void Base::gui_(ofxImGui::Settings & settings)
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

				if (ofxImGui::BeginTree(this->parameters.border, settings))
				{
					ofxImGui::AddParameter(this->parameters.border.width);
					ofxImGui::AddParameter(this->parameters.border.color);

					ofxImGui::EndTree(settings);
				}

				if (ofxImGui::BeginTree(this->parameters.transition, settings))
				{
					static vector<string> labels{ "Cut", "Mix", "Wipe", "Strobe" };
					ofxImGui::AddRadio(this->parameters.transition.type, labels, 2);

					if (static_cast<Transition>(this->parameters.transition.type.get()) != Transition::Cut)
					{
						ofxImGui::AddParameter(this->parameters.transition.duration);
					}

					ofxImGui::EndTree(settings);
				}

				if (ofxImGui::BeginTree(this->parameters.playback, settings))
				{
					ofxImGui::AddParameter(this->parameters.playback.fade);
					ofxImGui::AddParameter(this->parameters.playback.loop);
					static std::vector<std::string> syncLabels{ "Free Play", "Timeline", "Fade Control", "Linked Media" };
					ofxImGui::AddRadio(this->parameters.playback.syncMode, syncLabels, 2);

					ofxImGui::EndTree(settings);
				}
			}
			ofxImGui::EndWindow(settings);
		}

		//--------------------------------------------------------------
		void Base::serialize_(nlohmann::json & json)
		{
			json["type"] = static_cast<int>(this->type);

			ofxPreset::Serializer::Serialize(json, this->parameters);

			this->serialize(json);
		}

		//--------------------------------------------------------------
		void Base::deserialize_(const nlohmann::json & json)
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
		}

		//--------------------------------------------------------------
		bool Base::shouldPlay()
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

			const auto syncMode = static_cast<SyncMode>(this->parameters.playback.syncMode.get());
			if (syncMode == SyncMode::Timeline)
			{
				// No loop and switch is passed duration.
				if (!this->parameters.playback.loop && durationMs < this->switchMillis) return false;

				// Timeline is not playing.
				if (!this->timeline->getIsPlaying()) return false;
			}
			else if (syncMode == SyncMode::FreePlay)
			{
				uint64_t deltaMs;
				if (this->initFreePlay())
				{
					deltaMs = this->freePlayStartElapsedMs;
				}
				else
				{
					deltaMs = ofGetElapsedTimeMillis() - this->freePlayStartElapsedMs;
				}

				// No loop and time is passed duration.
				if (!this->parameters.playback.loop && durationMs < deltaMs) return false;
			}
			else if (syncMode == SyncMode::FadeControl)
			{
				// Fade value at 0.
				if (this->parameters.playback.fade == 0.0f) return false;

				uint64_t deltaMs;
				if (this->initFreePlay())
				{
					deltaMs = this->freePlayStartElapsedMs;
				}
				else
				{
					deltaMs = ofGetElapsedTimeMillis() - this->freePlayStartElapsedMs;
				}

				// No loop and time is passed duration.
				if (!this->parameters.playback.loop && durationMs < deltaMs) return false;
			}
			else // SyncMode::LinkedMedia
			{
				if (this->linkedMedia == nullptr) return false;

				return this->linkedMedia->shouldPlay();
			}

			return true;
		}

		//--------------------------------------------------------------
		void Base::addTimelineTrack()
		{
			if (!this->timeline)
			{
				ofLogError(__FUNCTION__) << "No timeline set, call init_() first!";
				return;
			}

			if (this->switchesTrack)
			{
				ofLogWarning(__FUNCTION__) << "Switches track already exists.";
				return;
			}
			
			// Add Page if it doesn't already exist.
			if (!this->timeline->hasPage(MediaTimelinePageName))
			{
				this->timeline->addPage(MediaTimelinePageName);
			}
			auto page = this->timeline->getPage(MediaTimelinePageName);

			const auto trackName = "Media_" + ofToString(this->index) + "_" + this->getTypeName();
			if (page->getTrack(trackName))
			{
				//ofLogWarning(__FUNCTION__) << "Track for Pop-up " << this->index << " already exists!";
				return;
			}

			this->timeline->setCurrentPage(MediaTimelinePageName);

			// Add Track.
			this->switchesTrack = this->timeline->addSwitches(trackName);
		}

		//--------------------------------------------------------------
		void Base::removeTimelineTrack()
		{
			if (!this->timeline)
			{
				//ofLogWarning(__FUNCTION__) << "No timeline set.";
				return;
			}

			if (!this->switchesTrack)
			{
				//ofLogWarning(__FUNCTION__) << "Switches track for Pop-up " << this->index << " does not exist.";
				return;
			}

			this->timeline->removeTrack(this->switchesTrack);
			this->switchesTrack = nullptr;

			// TODO: Figure out why this is not working!
			//auto page = this->timeline->getPage(kTimelinePageName);
			//if (page && page->getTracks().empty())
			//{
			//	cout << "Removing page " << page->getName() << endl;
			//	this->timeline->removePage(page);
			//}
		}

		//--------------------------------------------------------------
		bool Base::addDefaultSwitch()
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

			auto addedSwitch = this->switchesTrack->addSwitch(this->timeline->getInTimeInMillis(), this->getDurationMs());
			return (addedSwitch != nullptr);
		}

		//--------------------------------------------------------------
		void Base::updateBounds()
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

			this->borderDirty = true;

			this->boundsDirty = false;
		}

		//--------------------------------------------------------------
		void Base::updateBorder()
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
