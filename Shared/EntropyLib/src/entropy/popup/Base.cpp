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
		void Base::setup()
		{
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
		}

		//--------------------------------------------------------------
		void Base::draw()
		{
			ofPushStyle();
			{
				if (this->getParameters().base.background->a > 0)
				{
					ofSetColor(this->getParameters().base.background.get());
					ofDrawRectangle(this->viewport);
				}
		
				if (this->getTexture().isAllocated())
				{
					ofSetColor(ofColor::white);
					this->getTexture().drawSubsection(this->viewport.x, this->viewport.y, this->viewport.width, this->viewport.height,
													  this->roi.x, this->roi.y, this->roi.width, this->roi.height);
				}
			}
			ofPopStyle();
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
					// Add a section for the base parameters.
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
			json["type"] = this->type;

			ofxPreset::Serializer::Serialize(json, this->getParameters());

			this->onSerialize.notify(json);
		}

		//--------------------------------------------------------------
		void Base::deserialize(const nlohmann::json & json)
		{
			//int typeAsInt = json["type"];
			//this->type = (Type)typeAsInt; 
			
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