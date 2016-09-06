#include "Base.h"

#include "entropy/Helpers.h"

namespace entropy
{
	namespace popup
	{
		//--------------------------------------------------------------
		Base::Base(Type type)
			: type(type)
			, editing(false)
			, boundsDirty(false)
			, borderDirty(false)
			, transitionPct(0.0f)
			, switchMillis(-1.0f)
			, switchesTrack(nullptr)
		{}

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
		render::Layout Base::getLayout()
		{
			return static_cast<render::Layout>(this->getParameters().base.layout.get());
		}

		//--------------------------------------------------------------
		Surface Base::getSurface()
		{
			return static_cast<Surface>(this->getParameters().base.surface.get());
		}

		//--------------------------------------------------------------
		void Base::init_(int index, std::shared_ptr<ofxTimeline> timeline)
		{
			this->index = index;
			this->timeline = timeline;

			this->addTimelineTrack();

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

					static const ofxEasingQuad kEasingFunction;
					long transitionDuration = this->getParameters().transition.duration * 1000; 
					if (trackTime - activeSwitch->timeRange.min < transitionDuration)
					{
						// Transitioning in.
						this->transitionPct = ofxTween::map(trackTime, activeSwitch->timeRange.min, activeSwitch->timeRange.min + transitionDuration, 0.0f, 1.0f, true, kEasingFunction, ofxTween::easeOut);
						this->borderDirty = true;
					}
					else if (activeSwitch->timeRange.max - trackTime < transitionDuration)
					{
						// Transitioning out.
						this->transitionPct = ofxTween::map(trackTime, activeSwitch->timeRange.max - transitionDuration, activeSwitch->timeRange.max, 1.0f, 0.0f, true, kEasingFunction, ofxTween::easeIn);
						this->borderDirty = true;
					}
					else if (this->transitionPct != 1.0f)
					{
						this->transitionPct = 1.0f;
						this->borderDirty = true;
					}
				}
			}

			if (this->enabled || this->editing)
			{
				auto & parameters = this->getParameters();
				auto transition = static_cast<Transition>(parameters.transition.type.get());
				
				// Set front color value.
				if (this->enabled)
				{
					if (transition == Transition::Mix)
					{
						this->frontAlpha = this->transitionPct;
					}
					else if (transition == Transition::Strobe)
					{
						this->frontAlpha = (ofRandomuf() < this->transitionPct) ? 1.0f : 0.0f;
					}
					else
					{
						this->frontAlpha = 1.0f;
					}
				}
				else
				{
					this->frontAlpha = 0.5f;
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

				if (this->borderDirty && parameters.border.width > 0.0f)
				{
					// Rebuild the border mesh.
					this->borderMesh.clear();
					this->borderMesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);

					const auto borderBounds = ofRectangle(dstBounds.x - parameters.border.width, dstBounds.y - parameters.border.width, dstBounds.width + 2 * parameters.border.width, dstBounds.height + 2 * parameters.border.width);
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

					this->borderDirty = false;
				}
			}

			this->update(dt);
		}

		//--------------------------------------------------------------
		void Base::draw_()
		{
			auto & parameters = this->getParameters();

			ofPushStyle();
			{
				if (parameters.base.background->a > 0)
				{
					ofSetColor(parameters.base.background.get());
					ofDrawRectangle(this->viewport);
				}
		
				if (this->isLoaded() && (this->enabled || this->editing))
				{
					// Draw the border.
					if (parameters.border.width > 0.0f)
					{
						ofSetColor(parameters.border.color.get(), this->frontAlpha * 255);
						this->borderMesh.draw();
					}

					// Draw the content.
					ofEnableBlendMode(OF_BLENDMODE_ADD);
					ofSetColor(255, this->frontAlpha * 255);
					this->renderContent();
					ofEnableBlendMode(OF_BLENDMODE_ALPHA);
				}
			}
			ofPopStyle();

			this->draw();
		}

		//--------------------------------------------------------------
		void Base::gui_(ofxPreset::Gui::Settings & settings)
		{
			if (!this->editing) return;

			auto & parameters = this->getParameters();

			// Add a GUI window for the parameters.
			ofxPreset::Gui::SetNextWindow(settings);
			if (ofxPreset::Gui::BeginWindow("Pop-up " + ofToString(this->index) + ": " + parameters.getName(), settings, false, &this->editing))
			{
				// Add sections for the base parameters.
				if (ofxPreset::Gui::BeginTree(parameters.base, settings))
				{
					static std::vector<std::string> layoutLabels{ "Back", "Front" };
					if (ofxPreset::Gui::AddRadio(parameters.base.layout, layoutLabels, 2))
					{
						this->boundsDirty = true;
					}
					static std::vector<std::string> surfaceLabels{ "Base", "Overlay" };
					ofxPreset::Gui::AddRadio(parameters.base.surface, surfaceLabels, 2);
					ofxPreset::Gui::AddParameter(parameters.base.background);
					if (ofxPreset::Gui::AddParameter(parameters.base.size))
					{
						this->boundsDirty = true;
					}
					if (ofxPreset::Gui::AddParameter(parameters.base.center))
					{
						this->boundsDirty = true;
					}

					ofxPreset::Gui::EndTree(settings);
				}

				if (ofxPreset::Gui::BeginTree(parameters.border, settings))
				{
					if (ofxPreset::Gui::AddParameter(parameters.border.width))
					{
						this->borderDirty = true;
					}

					ofxPreset::Gui::AddParameter(parameters.border.color);

					ofxPreset::Gui::EndTree(settings);
				}

				if (ofxPreset::Gui::BeginTree(parameters.transition, settings))
				{
					static vector<string> labels{ "Cut", "Mix", "Wipe", "Strobe" };
					ofxPreset::Gui::AddRadio(parameters.transition.type, labels, 2);

					if (static_cast<Transition>(parameters.transition.type.get()) != Transition::Cut)
					{
						ofxPreset::Gui::AddParameter(parameters.transition.duration);
					}

					ofxPreset::Gui::EndTree(settings);
				}

				// Let the child class handle its child parameters.
				this->gui(settings);
			}
			ofxPreset::Gui::EndWindow(settings);
		}

		//--------------------------------------------------------------
		void Base::serialize_(nlohmann::json & json)
		{
			json["type"] = static_cast<int>(this->type);

			ofxPreset::Serializer::Serialize(json, this->getParameters());

			this->serialize(json);
		}

		//--------------------------------------------------------------
		void Base::deserialize_(const nlohmann::json & json)
		{
			ofxPreset::Serializer::Deserialize(json, this->getParameters());

			this->deserialize(json);

			this->boundsDirty = true;
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
			if (!this->timeline->hasPage(kPopUpsTimelinePageName))
			{
				this->timeline->addPage(kPopUpsTimelinePageName);
			}
			auto page = this->timeline->getPage(kPopUpsTimelinePageName);

			auto trackName = "_" + ofToString(this->index);
			if (this->type == Type::Image)
			{
				trackName.insert(0, "Image");
			}
			else if (this->type == Type::Video)
			{
				trackName.insert(0, "Video");
			}
			else if (this->type == Type::Sound)
			{
				trackName.insert(0, "Sound");
			}

			if (page->getTrack(trackName))
			{
				//ofLogWarning(__FUNCTION__) << "Track for Pop-up " << this->index << " already exists!";
				return;
			}

			this->timeline->setCurrentPage(kPopUpsTimelinePageName);

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
		void Base::updateBounds()
		{
			auto & parameters = this->getParameters();

			const auto layout = static_cast<render::Layout>(parameters.base.layout.get());
			const auto canvasSize = glm::vec2(GetCanvasWidth(layout), GetCanvasHeight(layout));
			const auto viewportHeight = canvasSize.y * parameters.base.size;
			if (this->isLoaded())
			{
				const auto contentRatio = this->getContentWidth() / this->getContentHeight();
				const auto viewportWidth = viewportHeight * contentRatio;
				this->viewport.setFromCenter(canvasSize * parameters.base.center.get(), viewportWidth, viewportHeight);

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
				this->viewport.setFromCenter(canvasSize * parameters.base.center.get(), viewportHeight, viewportHeight);
			}

			this->borderDirty = true;

			this->boundsDirty = false;
		}
	}
}