#include "Manager.h"

namespace entropy
{
	namespace scene
	{
		//--------------------------------------------------------------
		Manager::Manager()
		{
			// Register OF events.
			ofAddListener(ofEvents().update, this, &Manager::update);
			ofAddListener(ofEvents().draw, this, &Manager::draw);
			ofAddListener(ofEvents().keyPressed, this, &Manager::keyPressed);
		}

		//--------------------------------------------------------------
		Manager::~Manager()
		{
			// Unregister OF events.
			ofRemoveListener(ofEvents().update, this, &Manager::update);
			ofRemoveListener(ofEvents().draw, this, &Manager::draw);
			ofRemoveListener(ofEvents().keyPressed, this, &Manager::keyPressed);

			// Clear scenes.
			if (this->currentScene)
			{
				this->currentScene->exit();
			}
			this->currentScene.reset();
			this->scenes.clear();
		}

		//--------------------------------------------------------------
		bool Manager::addScene(shared_ptr<Base> scene)
		{
			if (this->scenes.find(scene->getName()) == this->scenes.end())
			{
				this->scenes.emplace(scene->getName(), scene);
				return true;
			}
			
			ofLogError("Manager::addScene") << "Scene with name " << scene->getName() << " already exists!";
			return false;
		}

		//--------------------------------------------------------------
		bool Manager::removeScene(shared_ptr<Base> scene)
		{
			return this->removeScene(scene->getName());
		}

		//--------------------------------------------------------------
		bool Manager::removeScene(const string & name)
		{
			if (this->scenes.erase(name))
			{
				return true;
			}

			ofLogError("Manager::removeScene") << "Scene with name " << name << " does not exist!";
			return false;
		}

		//--------------------------------------------------------------
		shared_ptr<Base> Manager::getScene(const string & name)
		{
			try
			{
				return this->scenes.at(name);
			}
			catch (const out_of_range &)
			{
				ofLogError("Manager::getScene") << "Scene with name " << name << " does not exist!";
				return nullptr;
			}
		}

		//--------------------------------------------------------------
		template<typename SceneType>
		shared_ptr<SceneType> Manager::getScene(const string & name)
		{
			auto scene = this->getScene(name);
			if (scene)
			{
				auto sceneTyped = dynamic_pointer_cast<SceneType>(scene);
				if (sceneTyped)
				{
					return sceneTyped;
				}
				ofLogError("Manager::getScene") << "Scene with name " << name << " does not match template!";
			}
			return nullptr;
		}

		//--------------------------------------------------------------
		shared_ptr<Base> Manager::getCurrentScene()
		{
			return this->currentScene;
		}

		//--------------------------------------------------------------
		template<typename SceneType>
		shared_ptr<SceneType> Manager::getCurrentScene()
		{
			if (this->currentScene)
			{
				auto sceneTyped = dynamic_pointer_cast<SceneType>(this->currentScene);
				if (sceneTyped)
				{
					return sceneTyped;
				}
				ofLogError("Manager::getCurrentScene") << "Current scene does not match template!";
			}
			return nullptr;
		}

		//--------------------------------------------------------------
		bool Manager::setCurrentScene(const string & name)
		{
			if (this->currentScene)
			{
				this->currentScene->exit();
			}
			
			auto scene = this->getScene(name);
			if (scene)
			{
				this->currentScene = scene;
				this->currentScene->setup();
				return true;
			}
			ofLogError("Manager::getScene") << "Scene with name " << name << " does not exist!";
			return false;
		}

		//--------------------------------------------------------------
		void Manager::update(ofEventArgs & args)
		{
			if (this->currentScene)
			{
				this->currentScene->update();

				if (this->currentScene->isOverlayVisible() /*|| this->parameters.camera.mouseEnabled*/)
				{
					ofShowCursor();
				}
				else
				{
					ofHideCursor();
				}
			}
		}

		//--------------------------------------------------------------
		void Manager::draw(ofEventArgs & args)
		{
			if (this->currentScene)
			{
				this->currentScene->draw();
			}
		}

		//--------------------------------------------------------------
		void Manager::keyPressed(ofKeyEventArgs & args)
		{
			if (args.key == OF_KEY_TAB)
			{
				ofToggleFullscreen();
			}
			else if (this->currentScene)
			{
				switch (args.key)
				{
				case '`':
					this->currentScene->toggleOverlayVisible();
					break;

				case 'L':
					this->currentScene->toggleCameraLocked();
					break;

				case 'T':
					this->currentScene->addCameraKeyframe();
					break;

				default:
					break;
				}
			}
		}
	}
}