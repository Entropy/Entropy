#include "Base.h"

#include "entropy/Helpers.h"

static const string kTimelinePageName = "Pop-Ups";

namespace entropy
{
	namespace popup
	{
		//--------------------------------------------------------------
		Base::Base(Type type)
			: type(type)
			, editing(false)
			, boundsDirty(false)
		{}

		//--------------------------------------------------------------
		Base::~Base()
		{}

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
		void Base::setup_(int index)
		{
			this->index = index;

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

			this->enabled = this->track->isOn();
			if (this->enabled)
			{
				// Update the transition if any switch is currently active.
				auto trackTime = this->track->currentTrackTime();
				auto activeSwitch = this->track->getActiveSwitchAtMillis(trackTime);
				if (activeSwitch)
				{
					static const ofxEasingQuad kEasingFunction;
					long transitionDuration = this->getParameters().transition.duration * 1000; 
					if (trackTime - activeSwitch->timeRange.min < transitionDuration)
					{
						// Transitioning in.
						this->transitionAmount = ofxTween::map(trackTime, activeSwitch->timeRange.min, activeSwitch->timeRange.min + transitionDuration, 0.0f, 1.0f, true, kEasingFunction, ofxTween::easeOut);
						this->borderDirty = true;
					}
					else if (activeSwitch->timeRange.max - trackTime < transitionDuration)
					{
						// Transitioning out.
						this->transitionAmount = ofxTween::map(trackTime, activeSwitch->timeRange.max - transitionDuration, activeSwitch->timeRange.max, 1.0f, 0.0f, true, kEasingFunction, ofxTween::easeIn);
						this->borderDirty = true;
					}
					else if (this->transitionAmount != 1.0f)
					{
						this->transitionAmount = 1.0f;
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
						this->frontAlpha = this->transitionAmount;
					}
					else if (transition == Transition::Strobe)
					{
						this->frontAlpha = (ofRandomuf() < this->transitionAmount) ? 1.0f : 0.0f;
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
				if (this->enabled && transition == Transition::Wipe && this->transitionAmount < 1.0f)
				{
					this->dstBounds.height = this->transitionAmount * this->viewport.height;
					this->dstBounds.y = this->viewport.y + (1.0f - this->transitionAmount) * this->viewport.height * 0.5f;

					this->srcBounds.height = this->transitionAmount * this->roi.height;
					this->srcBounds.y = this->roi.y + (1.0f - this->transitionAmount) * this->roi.height * 0.5f;
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
					ofSetColor(255, this->frontAlpha * 255);
					ofEnableBlendMode(OF_BLENDMODE_ADD);
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
			if (ofxPreset::Gui::BeginWindow("Pop-up: " + parameters.getName(), settings, false, &this->editing))
			{
				// Add sections for the base parameters.
				if (ImGui::CollapsingHeader(parameters.base.getName().c_str(), nullptr, true, true))
				{
					static vector<string> labels{ "Back", "Front" };
					ofxPreset::Gui::AddRadio(parameters.base.layout, labels, 2);
					ofxPreset::Gui::AddParameter(parameters.base.background);
					if (ofxPreset::Gui::AddParameter(parameters.base.size))
					{
						this->boundsDirty = true;
					}
					if (ofxPreset::Gui::AddParameter(parameters.base.center))
					{
						this->boundsDirty = true;
					}
				}

				if (ImGui::CollapsingHeader(parameters.border.getName().c_str(), nullptr, true, true))
				{
					if (ofxPreset::Gui::AddParameter(parameters.border.width))
					{
						this->borderDirty = true;
					}

					ofxPreset::Gui::AddParameter(parameters.border.color);
				}

				if (ImGui::CollapsingHeader(parameters.transition.getName().c_str(), nullptr, true, true))
				{
					static vector<string> labels{ "Cut", "Mix", "Wipe", "Strobe" };
					ofxPreset::Gui::AddRadio(parameters.transition.type, labels, 2);

					if (static_cast<Transition>(parameters.transition.type.get()) != Transition::Cut)
					{
						ofxPreset::Gui::AddParameter(parameters.transition.duration);
					}
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
		void Base::addTrack(ofxTimeline & timeline)
		{
			// Add Page if it doesn't already exist.
			if (!timeline.hasPage(kTimelinePageName))
			{
				timeline.addPage(kTimelinePageName);
			}
			auto page = timeline.getPage(kTimelinePageName);

			auto trackName = "_" + ofToString(this->index);
			if (this->type == Type::Image) trackName.insert(0, "Image");

			if (page->getTrack(trackName))
			{
				//ofLogWarning(__FUNCTION__) << "Track for Pop-Up " << this->index << " already exists!";
				return;
			}

			timeline.setCurrentPage(kTimelinePageName);

			// Add Track.
			this->track = timeline.addSwitches(trackName);
		}

		//--------------------------------------------------------------
		void Base::removeTrack(ofxTimeline & timeline)
		{
			if (!this->track)
			{
				//ofLogWarning(__FUNCTION__) << "Track for Pop-Up " << this->index << " does not exist!";
				return;
			}

			timeline.removeTrack(this->track);
			this->track = nullptr;

			auto page = timeline.getPage(kTimelinePageName);
			if (page && page->getTracks().empty())
			{
				timeline.removePage(page);
			}
		}

		//--------------------------------------------------------------
		void Base::updateBounds()
		{
			auto & parameters = this->getParameters();

			const auto layout = static_cast<render::Layout>(parameters.base.layout.get());
			const auto canvasSize = glm::vec2(GetCanvasWidth(layout), GetCanvasHeight(layout));
			this->viewport.setFromCenter(canvasSize * parameters.base.center.get(), canvasSize.x * parameters.base.size->x, canvasSize.y * parameters.base.size->y);
			
			if (this->isLoaded())
			{
				const auto contentRatio = this->getContentWidth() / this->getContentHeight();
				const auto viewportRatio = this->viewport.getAspectRatio();
				
				// Calculate the source subsection for Aspect Fill.
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

			this->borderDirty = true;

			this->boundsDirty = false;
		}
	}
}