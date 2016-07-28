#include "PopUps.h"

#include "entropy/popup/Image.h"

namespace entropy
{
	namespace scene
	{
		//--------------------------------------------------------------
		PopUps::PopUps()
			: Base()
		{
			ENTROPY_SCENE_SETUP_LISTENER;
		}
		
		//--------------------------------------------------------------
		PopUps::~PopUps()
		{

		}

		//--------------------------------------------------------------
		// Set up your crap here!
		void PopUps::setup()
		{
			ENTROPY_SCENE_EXIT_LISTENER;
			ENTROPY_SCENE_RESIZE_LISTENER;
			ENTROPY_SCENE_UPDATE_LISTENER;
			ENTROPY_SCENE_DRAW_BACK_LISTENER;
			ENTROPY_SCENE_DRAW_WORLD_LISTENER;
			ENTROPY_SCENE_DRAW_FRONT_LISTENER;
			ENTROPY_SCENE_GUI_LISTENER;
			ENTROPY_SCENE_SERIALIZATION_LISTENERS;
		}
		
		//--------------------------------------------------------------
		// Clean up your crap here!
		void PopUps::exit()
		{

		}

		//--------------------------------------------------------------
		// Resize your content here. 
		// Note that this is not the window size but the canvas size.
		void PopUps::resize(ofResizeEventArgs & args)
		{

		}

		//--------------------------------------------------------------
		// Update your data here, once per frame.
		void PopUps::update(double & dt)
		{

		}

		//--------------------------------------------------------------
		// Draw 2D elements in the background here.
		void PopUps::drawBack()
		{

		}
		
		//--------------------------------------------------------------
		// Draw 3D elements here.
		void PopUps::drawWorld()
		{

		}

		//--------------------------------------------------------------
		// Draw 2D elements in the foreground here.
		void PopUps::drawFront()
		{

		}

		//--------------------------------------------------------------
		// Add Scene specific GUI windows here.
		void PopUps::gui(ofxPreset::Gui::Settings & settings)
		{

		}

		//--------------------------------------------------------------
		// Do something after the parameters are saved.
		// You can save other stuff to the same json object here too.
		void PopUps::serialize(nlohmann::json & json)
		{

		}
		
		//--------------------------------------------------------------
		// Do something after the parameters are loaded.
		// You can load your other stuff here from that json object.
		// You can also set any refresh flags if necessary.
		void PopUps::deserialize(const nlohmann::json & json)
		{

		}
	}
} 