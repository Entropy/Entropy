#include "Video.h"

#include "entropy/Helpers.h"

namespace entropy
{
	namespace scene
	{
		//--------------------------------------------------------------
		Video::Video()
		{
			ENTROPY_SCENE_SETUP_LISTENER;
		}
		
		//--------------------------------------------------------------
		Video::~Video()
		{
			this->videoPlayer.close();
		}

		//--------------------------------------------------------------
		void Video::setup()
		{
			ENTROPY_SCENE_EXIT_LISTENER;
			ENTROPY_SCENE_RESIZE_LISTENER;
			ENTROPY_SCENE_UPDATE_LISTENER;
			ENTROPY_SCENE_DRAW_BACK_LISTENER;
			ENTROPY_SCENE_GUI_LISTENER;
			ENTROPY_SCENE_SERIALIZATION_LISTENERS;

			this->dirtyBounds = false;
		}
		
		//--------------------------------------------------------------
		void Video::exit()
		{

		}

		//--------------------------------------------------------------
		void Video::resize(ofResizeEventArgs & args)
		{
			this->dirtyBounds = true;
		}

		//--------------------------------------------------------------
		void Video::update(double & dt)
		{
			if (this->dirtyBounds)
			{
				if (this->parameters.centered)
				{
					this->videoBounds.setFromCenter(GetCanvasWidth() * 0.5f, GetCanvasHeight() * 0.5f, this->videoPlayer.getWidth(), this->videoPlayer.getHeight());
				}
				else
				{
					this->videoBounds.set(0.0f, 0.0f, this->videoPlayer.getWidth(), this->videoPlayer.getHeight());
				}

				this->dirtyBounds = false;
			}

			this->videoPlayer.update();
		}

		//--------------------------------------------------------------
		void Video::drawBack()
		{
			if (this->videoPlayer.isLoaded())
			{
				this->videoPlayer.draw(this->videoBounds.x, this->videoBounds.y, this->videoBounds.width, this->videoBounds.height);
			}
		}

		//--------------------------------------------------------------
		void Video::gui(ofxPreset::GuiSettings & settings)
		{
			ofxPreset::Gui::SetNextWindow(settings);
			if (ofxPreset::Gui::BeginWindow(this->parameters.getName(), settings))
			{
				if (ImGui::Button("Load File..."))
				{
					auto dialogResult = ofSystemLoadDialog("Load File", false, this->getDataPath("videos"));
					if (dialogResult.bSuccess)
					{
						this->loadVideo(dialogResult.filePath);
					}
				}
				if (!this->parameters.videoPath.get().empty())
				{
					ImGui::Text("File: %s", this->fileName);
				}

				if (ofxPreset::Gui::AddParameter(this->parameters.play))
				{
					if (this->parameters.play)
					{
						this->videoPlayer.play();
					}
					else
					{
						this->videoPlayer.stop();
					}
				}

				if (ofxPreset::Gui::AddParameter(this->parameters.loop))
				{
					if (this->parameters.loop)
					{
						this->videoPlayer.setLoopState(OF_LOOP_NORMAL);
					}
					else
					{
						this->videoPlayer.setLoopState(OF_LOOP_NONE);
					}
				}

				if (ofxPreset::Gui::AddParameter(this->parameters.centered))
				{
					this->dirtyBounds = true;
				}
			}
			ofxPreset::Gui::EndWindow(settings);
		}

		//--------------------------------------------------------------
		void Video::serialize(nlohmann::json & json)
		{

		}
		
		//--------------------------------------------------------------
		void Video::deserialize(const nlohmann::json & json)
		{
			if (!this->parameters.videoPath.get().empty())
			{
				this->loadVideo(this->parameters.videoPath);
			}
		}

		//--------------------------------------------------------------
		bool Video::loadVideo(const string & filePath)
		{
			if (!this->videoPlayer.load(filePath)) {
				ofLogError("Video::loadVideo") << "No video found at " << filePath;
				return false;
			}

			if (this->parameters.play)
			{
				this->videoPlayer.play();
			}

			if (this->parameters.loop)
			{
				this->videoPlayer.setLoopState(OF_LOOP_NORMAL);
			}
			else
			{
				this->videoPlayer.setLoopState(OF_LOOP_NONE);
			}

			this->parameters.videoPath = filePath;
			this->dirtyBounds = true;

			ofFile file = ofFile(filePath);
			this->fileName = file.getFileName();

			return true;
		}
	}
}