#include "Base.h"

namespace entropy
{
	namespace scene
	{
		//--------------------------------------------------------------
		Base::Base()
		{

		}

		//--------------------------------------------------------------
		Base::~Base()
		{

		}

		//--------------------------------------------------------------
		void Base::setup()
		{
			this->onSetup.notifyListeners();
		}

		//--------------------------------------------------------------
		void Base::exit()
		{
			this->onExit.notifyListeners();

			// Remove all listeners.
			this->onSetup.removeListeners(nullptr);
			this->onExit.removeListeners(nullptr);
			this->onUpdate.removeListeners(nullptr);
			this->onDraw.removeListeners(nullptr);
			this->onGui.removeListeners(nullptr);
			this->onSerialize.removeListeners(nullptr);
			this->onDeserialize.removeListeners(nullptr);
		}

		//--------------------------------------------------------------
		void Base::update()
		{
			this->onUpdate.notifyListeners();
		}

		//--------------------------------------------------------------
		void Base::draw()
		{
			this->onDraw.notifyListeners();
		}

		//--------------------------------------------------------------
		void Base::gui(ofxPreset::GuiSettings & settings)
		{
			auto & parameters = this->getParameters();

			ofxPreset::Gui::SetNextWindow(settings);
			if (ofxPreset::Gui::BeginWindow(parameters.base.getName(), settings))
			{
				ofxPreset::Gui::AddParameter(parameters.base.background);
			}
			ofxPreset::Gui::EndWindow(settings);

			this->onGui.notifyListeners(settings);
		}

		//--------------------------------------------------------------
		void Base::serialize(nlohmann::json & json)
		{
			this->onSerialize.notifyListeners(json);
		}

		//--------------------------------------------------------------
		void Base::deserialize(const nlohmann::json & json)
		{
			this->onDeserialize.notifyListeners(json);
		}
	}
}