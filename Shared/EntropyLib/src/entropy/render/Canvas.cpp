#include "Canvas.h"

#include "entropy/Helpers.h"

namespace entropy
{
	namespace render
	{
		//--------------------------------------------------------------
		Canvas::Canvas()
		{
			// Load default fullscreen fbo.
			this->fboSettings.width = ofGetWidth();
			this->fboSettings.height = ofGetHeight();
			this->fboSettings.textureTarget = GL_TEXTURE_2D;
			this->fbo.allocate(this->fboSettings);

			// Set ofxWarp shader path.
			string shaderPath = GetSharedDataPath();
			shaderPath = ofFilePath::addTrailingSlash(shaderPath.append("ofxWarp"));
			shaderPath = ofFilePath::addTrailingSlash(shaderPath.append("shaders"));
			ofxWarp::WarpBase::setShaderPath(shaderPath);

			// Load initial settings if any.
			if (!this->loadSettings())
			{
				for (int i = 0; i < MAX_NUM_WARPS; ++i)
				{
					this->openGuis[i] = false;
				}
				
				// Add a default warp to start with.
				this->addWarp(ofxWarp::WarpBase::TYPE_PERSPECTIVE_BILINEAR);
			}
		}

		//--------------------------------------------------------------
		Canvas::~Canvas()
		{
			this->setFillWindow(false);

			this->warps.clear();
			this->srcAreas.clear();
			this->overlaps.clear();
		}

		//--------------------------------------------------------------
		void Canvas::begin()
		{
			this->fbo.begin();
		}
		
		//--------------------------------------------------------------
		void Canvas::end()
		{
			this->fbo.end();
		}

		//--------------------------------------------------------------
		void Canvas::draw()
		{
			if (this->parameters.fillWindow)
			{
				// Draw the fbo texture directly.
				this->fbo.draw(0, 0);
			}
			else
			{
				// Go through warps and fbo texture subsections and draw the whole thing.
				for (auto i = 0; i < this->warps.size(); ++i)
				{
					this->warps[i]->draw(this->fbo.getTexture(), this->srcAreas[i]);
				}
			}
		}

		//--------------------------------------------------------------
		float Canvas::getWidth() const
		{
			return this->fbo.getWidth();
		}

		//--------------------------------------------------------------
		float Canvas::getHeight() const
		{
			return this->fbo.getHeight();
		}

		//--------------------------------------------------------------
		void Canvas::setWidth(float width)
		{
			if (this->fbo.getWidth() == width) return;

			this->fboSettings.width = width;
			this->fbo.allocate(this->fboSettings);

			ofResizeEventArgs args;
			args.width = this->fbo.getWidth();
			args.height = this->fbo.getHeight();
			this->resizeEvent.notify(args);
		}

		//--------------------------------------------------------------
		void Canvas::setHeight(float height)
		{
			if (this->fbo.getHeight() == height) return;

			this->fboSettings.height = height;
			this->fbo.allocate(this->fboSettings);

			ofResizeEventArgs args;
			args.width = this->fbo.getWidth();
			args.height = this->fbo.getHeight();
			this->resizeEvent.notify(args);
		}

		//--------------------------------------------------------------
		bool Canvas::getFillWindow() const
		{
			return this->parameters.fillWindow;
		}
		
		//--------------------------------------------------------------
		void Canvas::setFillWindow(bool fillWindow)
		{
			this->parameters.fillWindow.set(fillWindow);
		}

		//--------------------------------------------------------------
		void Canvas::addWarp(ofxWarp::WarpBase::Type type)
		{
			shared_ptr<ofxWarp::WarpBase> warp;
			string typeName;
			if (type == ofxWarp::WarpBase::TYPE_BILINEAR)
			{
				warp = make_shared<ofxWarp::WarpBilinear>();
				typeName = "Bilinear";
			}
			else if (type == ofxWarp::WarpBase::TYPE_PERSPECTIVE)
			{
				warp = make_shared<ofxWarp::WarpPerspective>();
				typeName = "Perspective";
			}
			else if (type == ofxWarp::WarpBase::TYPE_PERSPECTIVE_BILINEAR)
			{
				warp = make_shared<ofxWarp::WarpPerspectiveBilinear>();
				typeName = "Perspective Bilinear";
			}
			else
			{
				ofLogError("Canvas::addWarp") << "Unrecognized warp type " << type;
				return;
			}

			this->warps.push_back(warp);

			auto idx = this->warps.size() - 1;
			auto name = "Warp " + ofToString(idx) + ": " + typeName;

			this->warpParameters.push_back(WarpParameters());
			this->warpParameters.back().setName(name);

			this->openGuis[idx] = false;

			this->updateStitches();
		}
		
		//--------------------------------------------------------------
		void Canvas::removeWarp()
		{
			this->warps.pop_back();
			this->warpParameters.pop_back();

			this->updateStitches();
		}

		//--------------------------------------------------------------
		void Canvas::updateStitches()
		{
			const auto numWarps = this->warps.size();

			this->overlaps.resize(numWarps, 0.0f);
			
			// Calculate the total overlap to set the fbo width.
			auto totalOverlap = 0.0f;
			for (auto overlap : this->overlaps)
			{
				totalOverlap += overlap;
			}
			this->setWidth(ofGetWidth() - totalOverlap);

			// Update the areas.
			this->srcAreas.resize(numWarps);
			if (numWarps == 1)
			{
				// Take up all the entire area.
				const auto areaSize = ofVec2f(this->getWidth(), this->getHeight());
				this->warps[0]->setSize(areaSize);
				this->srcAreas[0] = ofRectangle(0.0f, 0.0f, areaSize.x, areaSize.y);
			}
			else
			{
				// Overlap stitches.
				const auto areaSize = ofVec2f(this->getWidth() / (float)numWarps, this->getHeight());
				auto currX = 0.0f;
				for (auto i = 0; i < numWarps; ++i)
				{
					this->warps[i]->setSize(areaSize);
					this->srcAreas[i] = ofRectangle(currX, 0.0f, areaSize.x, areaSize.y);
					currX = currX + areaSize.x - this->overlaps[i];
				}
			}
		}

		//--------------------------------------------------------------
		bool Canvas::isEditing() const
		{
			for (auto i = 0; i < this->warps.size(); ++i)
			{
				if (this->warps[i]->isEditing())
				{
					return true;
				}
			}

			return false;
		}

		//--------------------------------------------------------------
		void Canvas::drawGui(ofxPreset::GuiSettings & settings)
		{
			ofxPreset::Gui::SetNextWindow(settings);
			if (ofxPreset::Gui::BeginWindow("Canvas", settings))
			{
				if (ImGui::Button("Save"))
				{
					this->saveSettings();
				}
				ImGui::SameLine();
				if (ImGui::Button("Load"))
				{
					this->loadSettings();
				}

				ofxPreset::Gui::AddParameter(this->parameters.fillWindow);

				if (!this->parameters.fillWindow)
				{
					ImGui::ListBoxHeader("List", 3);
					for (auto i = 0; i < this->warps.size(); ++i)
					{
						auto name = "Warp " + ofToString(i);
						ImGui::Checkbox(name.c_str(), &this->openGuis[i]);
					}
					ImGui::ListBoxFooter();

					if (this->warps.size() < MAX_NUM_WARPS)
					{
						if (ImGui::Button("Add Warp..."))
						{
							ImGui::OpenPopup("Warps");
							ImGui::SameLine();
						}
						if (ImGui::BeginPopup("Warps"))
						{
							static vector<string> warpNames;
							if (warpNames.empty())
							{
								warpNames.push_back("Bilinear");
								warpNames.push_back("Perspective");
								warpNames.push_back("Perspective Bilinear");
							}
							for (auto i = 0; i < warpNames.size(); ++i)
							{
								if (ImGui::Selectable(warpNames[i].c_str()))
								{
									if (i == 0)
									{
										this->addWarp(ofxWarp::WarpBase::TYPE_BILINEAR);
									}
									else if (i == 1)
									{
										this->addWarp(ofxWarp::WarpBase::TYPE_PERSPECTIVE);
									}
									else
									{
										this->addWarp(ofxWarp::WarpBase::TYPE_PERSPECTIVE_BILINEAR);
									}
								}
							}
							ImGui::EndPopup();
						}
						ImGui::SameLine();
					}
					if (!this->warps.empty())
					{
						if (ImGui::Button("Remove Warp"))
						{
							this->removeWarp();
						}
					}
				}
			}
			ofxPreset::Gui::EndWindow(settings);

			auto warpSettings = settings;
			warpSettings.windowPos = ofVec2f(ofGetWidth() - warpSettings.windowSize.x - kGuiMargin * 3, kGuiMargin);
			for (auto i = 0; i < this->warps.size(); ++i)
			{
				if (this->openGuis[i])
				{
					auto warp = this->warps[i];
					auto & paramGroup = this->warpParameters[i];
					
					ofxPreset::Gui::SetNextWindow(warpSettings);
					if (ofxPreset::Gui::BeginWindow(paramGroup.getName(), warpSettings, false, &this->openGuis[i]))
					{
						if (ofxPreset::Gui::AddParameter(paramGroup.editing))
						{
							warp->setEditing(paramGroup.editing);
						}

						if (ofxPreset::Gui::AddParameter(paramGroup.brightness))
						{
							warp->setBrightness(paramGroup.brightness);
						}

						if (ImGui::Button("Flip X"))
						{
							warp->flipHorizontal();
						}
						ImGui::SameLine();
						if (ImGui::Button("Flip Y"))
						{
							warp->flipVertical();
						}

						if (warp->getType() != ofxWarp::WarpBase::TYPE_BILINEAR)
						{
							// Bilinear warp rotations are not implemented yet.
							if (ImGui::Button("Rotate CCW"))
							{
								warp->rotateCounterclockwise();
							}
							ImGui::SameLine();
							if (ImGui::Button("Rotate CW"))
							{
								warp->rotateClockwise();
							}
						}

						if (warp->getType() != ofxWarp::WarpBase::TYPE_PERSPECTIVE)
						{
							// The rest of the controls only apply to Bilinear warps.
							auto warpBilinear = dynamic_pointer_cast<ofxWarp::WarpBilinear>(warp);

							if (ofxPreset::Gui::AddParameter(paramGroup.linear))
							{
								warpBilinear->setLinear(paramGroup.linear);
							}

							ImGui::Text("Resolution: %i", warpBilinear->getResolution());
							if (ImGui::Button("<"))
							{
								warpBilinear->decreaseResolution();
							}
							ImGui::SameLine();
							if (ImGui::Button(">"))
							{
								warpBilinear->increaseResolution();
							}

							if (ofxPreset::Gui::AddParameter(paramGroup.adaptive))
							{
								warpBilinear->setAdaptive(paramGroup.adaptive);
							}

							auto numControlsX = warpBilinear->getNumControlsX();
							ImGui::Text("Num Controls X: %i", numControlsX);
							if (ImGui::Button("--x"))
							{
								warpBilinear->setNumControlsX((numControlsX + 1) / 2);
							}
							ImGui::SameLine();
							if (ImGui::Button("-x"))
							{
								warpBilinear->setNumControlsX(numControlsX - 1);
							}
							ImGui::SameLine();
							if (ImGui::Button("x+"))
							{
								warpBilinear->setNumControlsX(numControlsX + 1);
							}
							ImGui::SameLine();
							if (ImGui::Button("x++"))
							{
								warpBilinear->setNumControlsX((numControlsX * 2) - 1);
							}

							auto numControlsY = warpBilinear->getNumControlsY();
							ImGui::Text("Num Controls Y: %i", numControlsY);
							if (ImGui::Button("--y"))
							{
								warpBilinear->setNumControlsY((numControlsY + 1) / 2);
							}
							ImGui::SameLine();
							if (ImGui::Button("-y"))
							{
								warpBilinear->setNumControlsY(numControlsY - 1);
							}
							ImGui::SameLine();
							if (ImGui::Button("y+"))
							{
								warpBilinear->setNumControlsY(numControlsY + 1);
							}
							ImGui::SameLine();
							if (ImGui::Button("y++"))
							{
								warpBilinear->setNumControlsY((numControlsY * 2) - 1);
							}
						}
					}
					ofxPreset::Gui::EndWindow(warpSettings);
				}
			}

			settings.mouseOverGui |= warpSettings.mouseOverGui;
		}

		//--------------------------------------------------------------
		void Canvas::serialize(nlohmann::json & json)
		{
			// Serialize the warps.
			vector<nlohmann::json> jsonWarps;
			for (auto warp : this->warps)
			{
				nlohmann::json jsonWarp;
				warp->serialize(jsonWarp);
				jsonWarps.push_back(jsonWarp);
			}
			json["warps"] = jsonWarps;

			// Serialize the areas.
			vector<string> areas;
			for (auto & srcArea : this->srcAreas)
			{
				ostringstream oss;
				oss << srcArea;
				areas.push_back(oss.str());
			}
			json["areas"] = areas;

			// Serialize the parameters.
			ofxPreset::Serializer::Serialize(json, this->parameters);
		}
		
		//--------------------------------------------------------------
		void Canvas::deserialize(const nlohmann::json & json)
		{
			// Deserialize the warps.
			this->warps.clear();
			for (auto & jsonWarp : json["warps"])
			{
				shared_ptr<ofxWarp::WarpBase> warp;

				int typeAsInt = jsonWarp["type"];
				ofxWarp::WarpBase::Type type = (ofxWarp::WarpBase::Type)typeAsInt;
				switch (type)
				{
				case ofxWarp::WarpBase::TYPE_BILINEAR:
					warp = make_shared<ofxWarp::WarpBilinear>();
					break;

				case ofxWarp::WarpBase::TYPE_PERSPECTIVE:
					warp = make_shared<ofxWarp::WarpPerspective>();
					break;

				case ofxWarp::WarpBase::TYPE_PERSPECTIVE_BILINEAR:
					warp = make_shared<ofxWarp::WarpPerspectiveBilinear>();
					break;

				default:
					ofLogWarning("Canvas::deserialize") << "Unrecognized Warp type " << type;
				}

				if (warp)
				{
					warp->deserialize(jsonWarp);
					this->warps.push_back(warp);
				}
			}

			// Deserialize the areas.
			this->srcAreas.clear();
			for (const auto & jsonArea : json["areas"])
			{
				ofRectangle srcArea;
				istringstream iss;
				iss.str(jsonArea);
				iss >> srcArea;
				this->srcAreas.push_back(srcArea);
			}

			// Deserialize the parameters.
			ofxPreset::Serializer::Deserialize(json, this->parameters);
		}

		//--------------------------------------------------------------
		string Canvas::getSettingsFilePath()
		{
			static string filePath;
			if (filePath.empty())
			{
				filePath = GetSharedDataPath();
				filePath = ofFilePath::addTrailingSlash(filePath.append("entropy"));
				filePath = ofFilePath::addTrailingSlash(filePath.append("render"));
				filePath = ofFilePath::addTrailingSlash(filePath.append("Canvas"));
				filePath.append("settings.json");
			}
			return filePath;
		}

		//--------------------------------------------------------------
		bool Canvas::loadSettings()
		{
			auto filePath = this->getSettingsFilePath();
			auto file = ofFile(filePath, ofFile::ReadOnly);
			if (!file.exists())
			{
				ofLogWarning("Warp::loadSettings") << "File not found at path " << filePath;
				return false;
			}

			nlohmann::json json;
			file >> json;

			this->deserialize(json);

			return true;
		}
		
		//--------------------------------------------------------------
		bool Canvas::saveSettings()
		{
			nlohmann::json json;
			this->serialize(json);

			auto filePath = this->getSettingsFilePath();
			auto file = ofFile(filePath, ofFile::WriteOnly);
			file << json;

			return true;
		}

		//--------------------------------------------------------------
		bool Canvas::selectClosestControlPoint(const ofVec2f & pos)
		{
			size_t warpIdx = -1;
			size_t pointIdx = -1;
			static const auto maxDistance = 25.0f;
			auto distance = maxDistance;

			// Find warp and distance to closest control point.
			for (int i = this->warps.size() - 1; i >= 0; --i)
			{
				if (!this->warps[i]->isEditing())
				{
					continue;
				}

				float candidate;
				auto idx = this->warps[i]->findClosestControlPoint(pos, &candidate);
				if (candidate < distance)
				{
					distance = candidate;
					pointIdx = idx;
					warpIdx = i;
				}
			}

			focusedIndex = warpIdx;

			// Select the closest control point and deselect all others.
			for (int i = this->warps.size() - 1; i >= 0; --i)
			{
				if (i == this->focusedIndex)
				{
					this->warps[i]->selectControlPoint(pointIdx);
				}
				else
				{
					this->warps[i]->deselectControlPoint();
				}
			}

			return (focusedIndex != -1);
		}

		//--------------------------------------------------------------
		bool Canvas::cursorMoved(ofVec2f & pos)
		{
			return this->selectClosestControlPoint(pos);
		}

		//--------------------------------------------------------------
		bool Canvas::cursorDown(ofVec2f & pos)
		{
			this->selectClosestControlPoint(pos);

			if (this->focusedIndex < this->warps.size())
			{
				return this->warps[this->focusedIndex]->handleCursorDown(pos);
			}

			return false;
		}

		//--------------------------------------------------------------
		bool Canvas::cursorDragged(ofVec2f & pos)
		{
			if (this->focusedIndex < this->warps.size())
			{
				return this->warps[this->focusedIndex]->handleCursorDrag(pos);
			}

			return false;
		}

		//--------------------------------------------------------------
		bool Canvas::keyPressed(ofKeyEventArgs & args)
		{
			if (this->focusedIndex >= this->warps.size())
			{
				return false;
			}

			auto warp = this->warps[this->focusedIndex];

			if (args.key == OF_KEY_TAB)
			{
				// Select the next of previous (+ SHIFT) control point.
				size_t nextIndex;
				auto selectedIndex = warp->getSelectedControlPoint();
				if (ofGetKeyPressed(OF_KEY_SHIFT))
				{
					if (selectedIndex == 0)
					{
						nextIndex = warp->getNumControlPoints() - 1;
					}
					else
					{
						nextIndex = selectedIndex - 1;
					}
				}
				else
				{
					nextIndex = (selectedIndex + 1) % warp->getNumControlPoints();
				}
				warp->selectControlPoint(nextIndex);

				return true;
			}

			if (args.key == OF_KEY_UP || args.key == OF_KEY_DOWN || args.key == OF_KEY_LEFT || args.key == OF_KEY_RIGHT)
			{
				auto step = ofGetKeyPressed(OF_KEY_SHIFT) ? 10.0f : 0.5f;
				auto shift = ofVec2f::zero();
				if (args.key == OF_KEY_UP)
				{
					shift.y = -step / (float)ofGetHeight();
				}
				else if (args.key == OF_KEY_DOWN)
				{
					shift.y = step / (float)ofGetHeight();
				}
				else if (args.key == OF_KEY_LEFT)
				{
					shift.x = -step / (float)ofGetWidth();
				}
				else
				{
					shift.x = step / (float)ofGetWidth();
				}
				warp->moveControlPoint(warp->getSelectedControlPoint(), shift);

				return true;
			}

			return false;
		}

		//--------------------------------------------------------------
		void Canvas::windowResized(ofResizeEventArgs & args)
		{
			if (this->parameters.fillWindow)
			{
				if (this->fbo.getWidth() == args.width && this->fbo.getHeight() == args.height) return;

				this->fboSettings.width = args.width;
				this->fboSettings.height = args.height;
				this->fbo.allocate(this->fboSettings);

				this->resizeEvent.notify(args);
			}
			else
			{
				for (auto warp : this->warps)
				{
					warp->handleWindowResize(args.width, args.height);
				}
			}
		}
	}
}