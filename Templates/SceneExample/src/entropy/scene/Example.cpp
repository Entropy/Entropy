#include "Example.h"

#include "entropy/Helpers.h"

namespace entropy
{
	namespace scene
	{
		//--------------------------------------------------------------
		Example::Example()
			: Base()
		{}
		
		//--------------------------------------------------------------
		Example::~Example()
		{}

		//--------------------------------------------------------------
		// Set up your crap here!
		void Example::setup()
		{
			ofEnableLighting();
		}
		
		//--------------------------------------------------------------
		// Clean up your crap here!
		void Example::exit()
		{
			
		}

		//--------------------------------------------------------------
		// Resize your content here. 
		// Note that this is not the window size but the canvas size.
		void Example::resizeBack(ofResizeEventArgs & args)
		{

		}

		//--------------------------------------------------------------
		// Resize your content here. 
		// Note that this is not the window size but the canvas size.
		void Example::resizeFront(ofResizeEventArgs & args)
		{

		}

		//--------------------------------------------------------------
		// Update your data here, once per frame.
		void Example::update(double dt)
		{
			if (this->box.getSize().x != this->parameters.box.size)
			{
				this->box.set(this->parameters.box.size);
				light.setPosition(this->box.getSize() * 2);
			}
		}

		//--------------------------------------------------------------
		// Draw 2D elements in the background here.
		void Example::drawBackBase()
		{

		}
		
		//--------------------------------------------------------------
		// Draw 3D elements here.
		void Example::drawBackWorld()
		{
			this->drawScene(true);
		}

		//--------------------------------------------------------------
		// Draw 2D elements in the foreground here.
		void Example::drawBackOverlay()
		{
			this->drawOverlay(true, render::Layout::Back);
		}

		//--------------------------------------------------------------
		// Draw 2D elements in the background here.
		void Example::drawFrontBase()
		{

		}

		//--------------------------------------------------------------
		// Draw 3D elements here.
		void Example::drawFrontWorld()
		{
			this->drawScene(false);
		}

		//--------------------------------------------------------------
		// Draw 2D elements in the foreground here.
		void Example::drawFrontOverlay()
		{
			this->drawOverlay(false, render::Layout::Front);
		}

		//--------------------------------------------------------------
		void Example::drawScene(bool filled)
		{
			ofPushStyle();
			{
				ofEnableDepthTest();
				material.setDiffuseColor(this->parameters.box.color);
				light.enable();
				glEnable(GL_CULL_FACE);
				ofSetColor(this->parameters.box.color.get());
				material.begin();
				this->box.draw(filled ? OF_MESH_FILL : OF_MESH_WIREFRAME);
				glDisable(GL_CULL_FACE);
				material.end();
				light.disable();
				ofDisableDepthTest();
			}
			ofPopStyle();
		}

		//--------------------------------------------------------------
		void Example::drawOverlay(bool filled, render::Layout layout)
		{
			ofPushStyle();
			{
				filled ? ofFill() : ofNoFill();

				static const auto kBorderSize = 20.0f;
				const auto canvasWidth = GetCanvasWidth(layout);
				const auto canvasHeight = GetCanvasHeight(layout);

				ofSetColor(255, 0, 0, 128);
				ofDrawRectangle(0.0f, 0.0f, canvasWidth, kBorderSize);
				ofSetColor(0, 255, 0, 128);
				ofDrawRectangle(0.0f, canvasHeight - kBorderSize, canvasWidth, kBorderSize);
				ofSetColor(0, 0, 255, 128);
				ofDrawRectangle(0.0f, 0.0f, kBorderSize, canvasHeight);
				ofSetColor(0, 255, 255, 128);
				ofDrawRectangle(canvasWidth - kBorderSize, 0.0f, kBorderSize, canvasHeight);
			}
			ofPopStyle();
		}

		//--------------------------------------------------------------
		// Add Scene specific GUI windows here.
		void Example::gui(ofxPreset::Gui::Settings & settings)
		{
			ofxPreset::Gui::SetNextWindow(settings);
			if (ofxPreset::Gui::BeginWindow(this->parameters.getName(), settings))
			{
				if (ImGui::CollapsingHeader(this->parameters.box.getName().c_str(), nullptr, true, true))
				{
					ofxPreset::Gui::AddParameter(this->parameters.box.color);
					ofxPreset::Gui::AddParameter(this->parameters.box.size);
				}
			}
			ofxPreset::Gui::EndWindow(settings);
		}

		//--------------------------------------------------------------
		// Do something after the parameters are saved.
		// You can save other stuff to the same json object here too.
		void Example::serialize(nlohmann::json & json)
		{

		}
		
		//--------------------------------------------------------------
		// Do something after the parameters are loaded.
		// You can load your other stuff here from that json object.
		// You can also set any refresh flags if necessary.
		void Example::deserialize(const nlohmann::json & json)
		{

		}
	}
}