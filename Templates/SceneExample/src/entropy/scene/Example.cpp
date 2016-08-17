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
		void Example::resize(ofResizeEventArgs & args)
		{

		}

		//--------------------------------------------------------------
		// Update your data here, once per frame.
		void Example::update(double dt)
		{
			if (this->sphere.getSize().x != this->parameters.sphere.radius || this->sphere.getResolution().x != this->parameters.sphere.resolution)
			{
				this->sphere.set(this->parameters.sphere.radius);
				light.setPosition(this->sphere.getSize() * 2);
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
			ofPushStyle();
			{
				ofEnableDepthTest();
				material.setDiffuseColor(this->parameters.sphere.color);
				light.enable();
				glEnable(GL_CULL_FACE);
				ofSetColor(this->parameters.sphere.color.get());
				material.begin();
				if (this->parameters.sphere.filled)
				{
					this->sphere.draw(OF_MESH_FILL);
				}
				else
				{
					this->sphere.draw(OF_MESH_WIREFRAME);
				}
				glDisable(GL_CULL_FACE);
				material.end();
				light.disable();
				ofDisableDepthTest();
			}
			ofPopStyle();
		}

		//--------------------------------------------------------------
		// Draw 2D elements in the foreground here.
		void Example::drawBackOverlay()
		{
			//static const float kBorderSize = 20.0f;
			//ofSetColor(255, 0, 0, 128);
			//ofDrawRectangle(0.0f, 0.0f, GetCanvasWidth(), kBorderSize);
			//ofSetColor(0, 255, 0, 128);
			//ofDrawRectangle(0.0f, GetCanvasHeight() - kBorderSize, GetCanvasWidth(), kBorderSize);
			//ofSetColor(0, 0, 255, 128);
			//ofDrawRectangle(0.0f, 0.0f, kBorderSize, GetCanvasHeight());
			//ofSetColor(0, 255, 255, 128);
			//ofDrawRectangle(GetCanvasWidth() - kBorderSize, 0.0f, kBorderSize, GetCanvasHeight());
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
			//ofPushStyle();
			//{
			//	ofEnableDepthTest();
			//	material.setDiffuseColor(this->parameters.sphere.color);
			//	light.enable();
			//	glEnable(GL_CULL_FACE);
			//	ofSetColor(this->parameters.sphere.color.get());
			//	material.begin();
			//	if (this->parameters.sphere.filled)
			//	{
			//		this->sphere.draw(OF_MESH_FILL);
			//	}
			//	else
			//	{
			//		this->sphere.draw(OF_MESH_WIREFRAME);
			//	}
			//	glDisable(GL_CULL_FACE);
			//	material.end();
			//	light.disable();
			//	ofDisableDepthTest();
			//}
			//ofPopStyle();
		}

		//--------------------------------------------------------------
		// Draw 2D elements in the foreground here.
		void Example::drawFrontOverlay()
		{
			static const auto kBorderSize = 20.0f;
			const auto canvasWidth = GetCanvasFrontWidth();
			const auto canvasHeight = GetCanvasFrontHeight();

			ofSetColor(255, 0, 0, 128);
			ofDrawRectangle(0.0f, 0.0f, canvasWidth, kBorderSize);
			ofSetColor(0, 255, 0, 128);
			ofDrawRectangle(0.0f, canvasHeight - kBorderSize, canvasWidth, kBorderSize);
			ofSetColor(0, 0, 255, 128);
			ofDrawRectangle(0.0f, 0.0f, kBorderSize, canvasHeight);
			ofSetColor(0, 255, 255, 128);
			ofDrawRectangle(canvasWidth - kBorderSize, 0.0f, kBorderSize, canvasHeight);
		}

		//--------------------------------------------------------------
		// Add Scene specific GUI windows here.
		void Example::gui(ofxPreset::Gui::Settings & settings)
		{
			ofxPreset::Gui::SetNextWindow(settings);
			if (ofxPreset::Gui::BeginWindow(this->parameters.getName(), settings))
			{
				if (ImGui::CollapsingHeader(this->parameters.sphere.getName().c_str(), nullptr, true, true))
				{
					ofxPreset::Gui::AddParameter(this->parameters.sphere.color);
					ofxPreset::Gui::AddParameter(this->parameters.sphere.filled);
					ofxPreset::Gui::AddParameter(this->parameters.sphere.radius);
					ofxPreset::Gui::AddParameter(this->parameters.sphere.resolution);
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