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
		Base::Type Base::getType() const
		{
			return this->type;
		}

		//--------------------------------------------------------------
		void Base::setup(int index)
		{
			this->index = index;

			this->onSetup.notify();

			this->boundsDirty = true;
		}

		//--------------------------------------------------------------
		void Base::exit()
		{
			this->onExit.notify();
		}

		//--------------------------------------------------------------
		void Base::resize(ofResizeEventArgs & args)
		{
			// Update right away so that event listeners can use the new bounds.
			this->updateBounds();
			
			this->onResize.notify(args);
		}

		//--------------------------------------------------------------
		void Base::update(double dt)
		{
			if (this->boundsDirty)
			{
				this->updateBounds();
			}

			this->enabled = this->track->isOn();
			if (this->enabled)
			{
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
					}
					else if (activeSwitch->timeRange.max - trackTime < transitionDuration)
					{
						// Transitioning out.
						this->transitionAmount = ofxTween::map(trackTime, activeSwitch->timeRange.max - transitionDuration, activeSwitch->timeRange.max, 1.0f, 0.0f, true, kEasingFunction, ofxTween::easeIn);
					}
					else
					{
						this->transitionAmount = 1.0f;
					}
				}
			}
		}

		//--------------------------------------------------------------
		void Base::draw()
		{
			auto & parameters = this->getParameters();
			
			ofPushStyle();
			{
				if (parameters.base.background->a > 0)
				{
					ofSetColor(parameters.base.background.get());
					ofDrawRectangle(this->viewport);
				}
		
				if (this->getTexture().isAllocated() && (this->enabled || this->editing))
				{
					auto transition = static_cast<Transition>(parameters.transition.type.get());

					// Set alpha value.
					float alpha;
					if (this->enabled)
					{
						if (transition == Transition::Mix)
						{
							alpha = this->transitionAmount;
						}
						else
						{
							alpha = 1.0f;
						}
					}
					else
					{
						alpha = 0.5f;
					}

					// Set subsection.
					auto dstBounds = this->viewport;
					auto srcBounds = this->roi;
					if (this->enabled && transition == Transition::Wipe && this->transitionAmount < 1.0f)
					{
						dstBounds.height = this->transitionAmount * this->viewport.height;
						dstBounds.y = this->viewport.y + (1.0f - this->transitionAmount) * this->viewport.height * 0.5f;

						srcBounds.height = this->transitionAmount * this->roi.height;
						srcBounds.y = this->roi.y + (1.0f - this->transitionAmount) * this->roi.height * 0.5f;
					}

					// Draw the texture.
					ofSetColor(ofColor::white, 255 * alpha);
					this->getTexture().drawSubsection(dstBounds.x, dstBounds.y, dstBounds.width, dstBounds.height,
													  srcBounds.x, srcBounds.y, srcBounds.width, srcBounds.height);
				}
			}
			ofPopStyle();
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
		void Base::gui(ofxPreset::Gui::Settings & settings)
		{
			if (!this->editing) return;

			auto & parameters = this->getParameters();

			if (this->onGuiListeners.size())
			{
				// Add a GUI window for the parameters.
				ofxPreset::Gui::SetNextWindow(settings);
				if (ofxPreset::Gui::BeginWindow("Pop-up: " + parameters.getName(), settings, false, &this->editing))
				{
					// Add sections for the base parameters.
					if (ImGui::CollapsingHeader(parameters.base.getName().c_str(), nullptr, true, true))
					{
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

					if (ImGui::CollapsingHeader(parameters.transition.getName().c_str(), nullptr, true, true))
					{
						ImGui::Columns(3);
						ImGui::RadioButton("Cut", parameters.transition.type.getRef(), static_cast<int>(Transition::Cut)); ImGui::NextColumn();
						ImGui::RadioButton("Mix", parameters.transition.type.getRef(), static_cast<int>(Transition::Mix)); ImGui::NextColumn();
						ImGui::RadioButton("Wipe", parameters.transition.type.getRef(), static_cast<int>(Transition::Wipe)); ImGui::NextColumn();
						ImGui::Columns(1);

						if (static_cast<Transition>(parameters.transition.type.get()) != Transition::Cut)
						{
							ofxPreset::Gui::AddParameter(parameters.transition.duration);
						}
					}

					// Let the child class handle its child parameters.
					this->onGui.notify(settings);
				}
				ofxPreset::Gui::EndWindow(settings);
			}
			else
			{
				// Build a default GUI for all parameters.
				ofxPreset::Gui::SetNextWindow(settings);
				ofxPreset::Gui::AddGroup(parameters, settings);
			}
		}

		//--------------------------------------------------------------
		void Base::serialize(nlohmann::json & json)
		{
			json["type"] = static_cast<int>(this->type);

			ofxPreset::Serializer::Serialize(json, this->getParameters());

			this->onSerialize.notify(json);
		}

		//--------------------------------------------------------------
		void Base::deserialize(const nlohmann::json & json)
		{
			ofxPreset::Serializer::Deserialize(json, this->getParameters());

			this->onDeserialize.notify(json);

			this->boundsDirty = true;
		}

		//--------------------------------------------------------------
		void Base::updateBounds()
		{
			const auto canvasSize = glm::vec2(GetCanvasWidth(), GetCanvasHeight());
			this->viewport.setFromCenter(canvasSize * this->getParameters().base.center.get(), canvasSize.x * this->getParameters().base.size->x, canvasSize.y * this->getParameters().base.size->y);
			
			if (this->getTexture().isAllocated())
			{
				const auto contentRatio = this->getTexture().getWidth() / this->getTexture().getHeight();
				const auto viewportRatio = this->viewport.getAspectRatio();
				
				// Calculate the source subsection for Aspect Fill.
				if (this->viewport.getAspectRatio() > contentRatio)
				{
					this->roi.width = this->getTexture().getWidth();
					this->roi.height = this->roi.width / viewportRatio;
					this->roi.x = 0.0f;
					this->roi.y = (this->getTexture().getHeight() - this->roi.height) * 0.5f;
				}
				else
				{
					this->roi.height = this->getTexture().getHeight();
					this->roi.width = this->roi.height * viewportRatio;
					this->roi.y = 0.0f;
					this->roi.x = (this->getTexture().getWidth() - this->roi.width) * 0.5f;
				}
			}

			this->boundsDirty = false;
		}
	}
}