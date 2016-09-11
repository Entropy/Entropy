#include "Canvas.h"

#include "entropy/Helpers.h"

namespace entropy
{
	namespace render
	{
		//--------------------------------------------------------------
		Canvas::Canvas(Layout layout)
			: layout(layout)
			, exportFrames(false)
		{
			this->parameters.setName((layout == Layout::Back) ? "Canvas Back" : "Canvas Front");
			
			// Set ofxWarp shader path.
			auto warpShaderPath = GetSharedDataPath();
			warpShaderPath = ofFilePath::addTrailingSlash(warpShaderPath.append("ofxWarp"));
			warpShaderPath = ofFilePath::addTrailingSlash(warpShaderPath.append("shaders"));
			ofxWarp::WarpBase::setShaderPath(warpShaderPath);

			// Set default fbo settings.
			this->fboSettings.width = ofGetWidth();
			this->fboSettings.height = ofGetHeight();
			this->fboSettings.numSamples = 4;
			this->fboSettings.internalformat = GL_RGBA16F;
			this->fboSettings.textureTarget = GL_TEXTURE_2D;

			this->fboDraw.allocate(this->fboSettings);

			this->fboSettings.numSamples = 0;
			this->fboPost.allocate(this->fboSettings);

			// Update viewport.
			this->viewport = ofRectangle(0.0f, 0.0f, this->getWidth(), this->getHeight());

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

			this->dirtyStitches = true;
		}

		//--------------------------------------------------------------
		Canvas::~Canvas()
		{
			this->setFillWindow(false);

			this->warps.clear();
			this->srcAreas.clear();
		}

		//--------------------------------------------------------------
		void Canvas::update()
		{
			if (this->dirtyStitches)
			{
				this->updateStitches();
			}

			// Reset post-processing flag, it will be set in postProcess() if called.
			this->postApplied = false;
		}

		//--------------------------------------------------------------
		void Canvas::beginDraw()
		{
			if (this->postApplied)
			{
				this->fboPost.begin();
			}
			else
			{
				this->fboDraw.begin(ofFboBeginMode::Perspective);
			}
		}

		//--------------------------------------------------------------
		void Canvas::endDraw()
		{
			if (this->postApplied)
			{
				this->fboPost.end();
			}
			else
			{
				this->fboDraw.end();
			}
		}

		//--------------------------------------------------------------
        void Canvas::postProcess(PostParameters & parameters)
		{
            this->postEffects.process(this->fboDraw.getTexture(), this->fboPost, parameters);
			this->postApplied = true;
		}

		//--------------------------------------------------------------
		void Canvas::render(const ofRectangle & bounds)
		{
			auto & texture = this->getRenderTexture();

			if (this->parameters.fillWindow)
			{
				// Draw the fbo texture directly.
				texture.draw(bounds);
			}
			else
			{
				ofPushMatrix();
				ofTranslate(bounds.x, bounds.y);
				{
					// Go through warps and fbo texture subsections and draw the whole thing.
					for (auto i = 0; i < this->warps.size(); ++i)
					{
						if (this->warpParameters[i].enabled)
						{
							this->warps[i]->draw(texture, this->srcAreas[i]);
						}
					}
				}
				ofPopMatrix();
			}

			if (this->exportFrames)
			{
				auto scene = GetCurrentScene();
				if (scene)
				{
					this->textureRecorder.save(texture, scene->getCurrentTimelineFrame());
				}
				else
				{
					// Nope, no Scene.
					this->exportFrames = false;
				}
			}
		}

		//--------------------------------------------------------------
		const ofTexture & Canvas::getDrawTexture() const
		{
			return this->fboDraw.getTexture();
		}

		//--------------------------------------------------------------
		const ofFbo & Canvas::getPostFbo() const
		{
			return this->fboPost;
		}

		//--------------------------------------------------------------
		const ofTexture & Canvas::getRenderTexture() const
		{
			return (this->postApplied ? this->fboPost.getTexture() : this->fboDraw.getTexture());
		}

		//--------------------------------------------------------------
		float Canvas::getWidth() const
		{
			return this->fboDraw.getWidth();
		}

		//--------------------------------------------------------------
		float Canvas::getHeight() const
		{
			return this->fboDraw.getHeight();
		}

		//--------------------------------------------------------------
		const ofRectangle & Canvas::getViewport() const
		{
			return this->viewport;
		}

		//--------------------------------------------------------------
		void Canvas::setWidth(float width)
		{
			if (this->fboDraw.getWidth() == width) return;

			this->fboSettings.width = width;
			this->updateSize();
		}

		//--------------------------------------------------------------
		void Canvas::setHeight(float height)
		{
			if (this->fboDraw.getHeight() == height) return;

			this->fboSettings.height = height;
			this->updateSize();
		}

		//--------------------------------------------------------------
		void Canvas::updateSize()
		{
			//ofLogNotice(__FUNCTION__) << "FBO dimensions " << this->fboSettings.width << " x " << this->fboSettings.height;
			
			// Re-allocate fbos.
			this->fboSettings.numSamples = 4;
			this->fboSettings.internalformat = GL_RGBA16F;
			this->fboSettings.textureTarget = GL_TEXTURE_2D;

			this->fboDraw.allocate(this->fboSettings);
			//this->fboDraw.getTexture().texData.bFlipTexture = true;

			this->fboSettings.numSamples = 0;			
			this->fboPost.allocate(this->fboSettings);
			//this->fboPost.getTexture().texData.bFlipTexture = true;

			// Update viewport.
			this->viewport = ofRectangle(0.0f, 0.0f, this->getWidth(), this->getHeight());

			// Update post effects.
			this->postEffects.resize(this->getWidth(), this->getHeight());

			// Update all existing warps.
			for (auto warp : this->warps)
			{
				warp->setSize(this->getWidth(), this->getHeight());
			}

			// Notify any listeners.
			ofResizeEventArgs args;
			args.width = this->getWidth();
			args.height = this->getHeight();
			this->resizeEvent.notify(args);

			this->dirtyStitches = true;
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
		shared_ptr<ofxWarp::WarpBase> Canvas::addWarp(ofxWarp::WarpBase::Type type)
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
				ofLogError(__FUNCTION__) << "Unrecognized warp type " << type;
				return nullptr;
			}

			warp->setSize(this->getWidth(), this->getHeight());
			warp->handleWindowResize(this->screenWidth, this->screenHeight);

			this->warps.push_back(warp);

			auto idx = this->warps.size() - 1;
			auto name = "Warp " + ofToString(idx) + ": " + typeName;

			this->warpParameters.push_back(WarpParameters());
			this->warpParameters.back().setName(name);

			this->openGuis[idx] = false;

			this->dirtyStitches = true;

			return warp;
		}
		
		//--------------------------------------------------------------
		void Canvas::removeWarp()
		{
			this->warps.pop_back();
			this->warpParameters.pop_back();

			this->dirtyStitches = true;
		}

		//--------------------------------------------------------------
		void Canvas::updateStitches()
		{
			const auto numWarps = this->warps.size();

			this->srcAreas.resize(numWarps);

			if (numWarps == 1)
			{
				// Take up the full size.
				this->srcAreas[0] = ofRectangle(0.0f, 0.0f, this->getWidth(), this->getHeight());
			}
			else if (numWarps > 1)
			{
				vector<float> overlaps;
				overlaps.resize(numWarps, 0.0f);

				// Calculate the overlap for each stitch and the total overlap.
				const auto areaSize = glm::vec2(this->screenWidth / static_cast<float>(numWarps), this->getHeight());
				auto totalOverlap = 0.0f;
				for (auto i = 0; i < numWarps - 1; ++i)
				{
					auto warp = this->warps[i];
					auto rightEdge = warp->getEdges().z;
					overlaps[i] = areaSize.x * rightEdge * 0.5f;
					totalOverlap += overlaps[i];
				}

				// Adjust the fbo width.
				this->setWidth(this->screenWidth - totalOverlap);

				// Update the areas for each warp.
				auto accOverlap = 0.0f;
				for (auto i = 0; i < numWarps; ++i)
				{
					const auto offsetLeft = i * areaSize.x - accOverlap;
					const auto offsetRight = offsetLeft + areaSize.x;
					this->srcAreas[i] = ofRectangle(offsetLeft, 0.0f, offsetRight - offsetLeft, areaSize.y);

					accOverlap += overlaps[i];
				}
			}

			this->dirtyStitches = false;
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
		void Canvas::drawGui(ofxPreset::Gui::Settings & settings)
		{
			ofxPreset::Gui::SetNextWindow(settings);
			if (ofxPreset::Gui::BeginWindow(this->parameters.getName().c_str(), settings))
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
				
				if (ofxPreset::Gui::BeginTree("Render", settings))
				{
					if (ImGui::Checkbox("Export", &this->exportFrames))
					{
						auto scene = GetCurrentScene();
						if (scene)
						{
							if (this->exportFrames)
							{
								// Build with a suggested name for the export folder.
								ostringstream oss;
								auto tokens = ofSplitString(scene->getName(), "::", true, true);
								oss << tokens.back() << "-" << scene->getCurrentPresetName() << "-" << ofGetTimestampString("%Y%m%d-%H%M%S");
								auto folderName = ofSystemTextBoxDialog("Save to folder", oss.str());
								if (folderName.length())
								{
									auto exportPath = ofFilePath::addTrailingSlash(GetSharedExportsPath().append(folderName));
									this->textureRecorder.setup(this->getWidth(), this->getHeight(), OF_PIXELS_RGBA, OF_IMAGE_FORMAT_PNG, exportPath);
									scene->beginExport();
								}
								else
								{
									// Nope, no folder selected.
									this->exportFrames = false;
								}
							}
							else
							{
								scene->endExport();
								this->exportFrames = false;
							}
						}
						else
						{
							// Nope, no Scene.
							this->exportFrames = false;
						}
					}

					ofxPreset::Gui::EndTree(settings);
				}
				
				if (ofxPreset::Gui::BeginTree("Warping", settings))
				{
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
								static vector<string> warpNames = 
								{ 
									"Bilinear", 
									"Perspective", 
									"Perspective Bilinear"
								};
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

					ofxPreset::Gui::EndTree(settings);
				}
			}
			ofxPreset::Gui::EndWindow(settings);

			// Move to the next column for the Warp gui windows.
			auto warpSettings = ofxPreset::Gui::Settings();
			//warpSettings.windowPos = glm::vec2(settings.totalBounds.getMaxX() + kGuiMargin, 0.0f);
			warpSettings.windowPos = glm::vec2(800.0f + kGuiMargin, 0.0f);
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

						ofxPreset::Gui::AddParameter(paramGroup.enabled);

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

							if (ofxPreset::Gui::AddParameter(paramGroup.mesh.linear))
							{
								warpBilinear->setLinear(paramGroup.mesh.linear);
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

							if (ofxPreset::Gui::AddParameter(paramGroup.mesh.adaptive))
							{
								warpBilinear->setAdaptive(paramGroup.mesh.adaptive);
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

						if (this->warps.size() > 1)
						{
							if (ofxPreset::Gui::BeginTree(paramGroup.blend, settings))
							{
								auto tmpLuminanceRef = paramGroup.blend.luminance.get();
								if (ImGui::ColorEdit3(paramGroup.blend.luminance.getName().c_str(), glm::value_ptr(tmpLuminanceRef)))
								{
									paramGroup.blend.luminance.set(tmpLuminanceRef);
									warp->setLuminance(paramGroup.blend.luminance);
								}
								auto tmpGammaRef = paramGroup.blend.gamma.get();
								if (ImGui::ColorEdit3(paramGroup.blend.gamma.getName().c_str(), glm::value_ptr(tmpGammaRef)))
								{
									paramGroup.blend.gamma.set(tmpGammaRef);
									warp->setGamma(paramGroup.blend.gamma);
								}
								if (ofxPreset::Gui::AddParameter(paramGroup.blend.exponent))
								{
									warp->setExponent(paramGroup.blend.exponent);
								}
								if (i > 0)
								{
									if (ofxPreset::Gui::AddParameter(paramGroup.blend.edgeLeft))
									{
										// Set current left edge.
										auto edgesSelf = warp->getEdges();
										edgesSelf.x = paramGroup.blend.edgeLeft;
										warp->setEdges(edgesSelf);

										// Set previous right edge.
										auto warpPrev = this->warps[i - 1];
										auto edgesPrev = warpPrev->getEdges();
										edgesPrev.z = paramGroup.blend.edgeLeft;
										warpPrev->setEdges(edgesPrev);

										// Sync previous parameters.
										auto paramsPrev = this->warpParameters[i - 1];
										paramsPrev.blend.edgeRight = paramGroup.blend.edgeLeft;

										this->dirtyStitches = true;
									}
								}
								if (i < this->warps.size() - 1)
								{
									if (ofxPreset::Gui::AddParameter(paramGroup.blend.edgeRight))
									{
										// Set current right edge.
										auto edges = warp->getEdges();
										edges.z = paramGroup.blend.edgeRight;
										warp->setEdges(edges);

										// Set next left edge.
										auto warpNext = this->warps[i + 1];
										auto edgeNext = warpNext->getEdges();
										edgeNext.x = paramGroup.blend.edgeRight;
										warpNext->setEdges(edgeNext);

										// Sync next parameters.
										auto paramsNext = this->warpParameters[i + 1];
										paramsNext.blend.edgeLeft = paramGroup.blend.edgeRight;

										this->dirtyStitches = true;
									}
								}

								ofxPreset::Gui::EndTree(settings);
							}
						}
					}
					ofxPreset::Gui::EndWindow(warpSettings);
				}
			}

			settings.totalBounds.growToInclude(warpSettings.totalBounds);
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
			this->warpParameters.clear();
			for (auto & jsonWarp : json["warps"])
			{
				int typeAsInt = jsonWarp["type"];
				ofxWarp::WarpBase::Type type = (ofxWarp::WarpBase::Type)typeAsInt;
				
				auto warp = this->addWarp(type);
				if (warp)
				{
					warp->deserialize(jsonWarp);

					// Sync warp with corresponding parameter group.
					auto & warpParams = warpParameters.back();
					warpParams.brightness = warp->getBrightness();

					warpParams.blend.luminance = warp->getLuminance();
					warpParams.blend.gamma = warp->getGamma();
					warpParams.blend.exponent = warp->getExponent();
					warpParams.blend.edgeLeft = warp->getEdges().x;
					warpParams.blend.edgeRight = warp->getEdges().z;

					if (warp->getType() != ofxWarp::WarpBase::TYPE_PERSPECTIVE)
					{
						auto warpBilinear = dynamic_pointer_cast<ofxWarp::WarpBilinear>(warp);
						warpParams.mesh.adaptive.set(warpBilinear->getAdaptive());
						warpParams.mesh.linear.set(warpBilinear->getLinear());
					}
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
		Layout Canvas::getLayout() const
		{
			return this->layout;
		}

		//--------------------------------------------------------------
        std::filesystem::path Canvas::getSettingsFilePath()
		{
            auto file = std::filesystem::path((this->layout == Layout::Back) ? "Back.json" : "Front.json");
            return GetDataPath(Module::Canvas) / file;
		}

		//--------------------------------------------------------------
        std::filesystem::path Canvas::getShaderPath(const string & shaderFile)
		{
            return GetShadersPath(Module::Canvas) / shaderFile;
		}

		//--------------------------------------------------------------
		bool Canvas::loadSettings()
		{
			auto filePath = this->getSettingsFilePath();
			auto file = ofFile(filePath, ofFile::ReadOnly);
			if (!file.exists())
			{
				ofLogWarning(__FUNCTION__) << "File not found at path " << filePath;
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
			file << json.dump(4);

			return true;
		}

		//--------------------------------------------------------------
		bool Canvas::selectClosestControlPoint(const glm::vec2 & pos)
		{
			size_t warpIdx = -1;
			size_t pointIdx = -1;
			static const auto maxDistance = numeric_limits<float>::max();
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
		bool Canvas::cursorMoved(const glm::vec2 & pos)
		{
			return this->selectClosestControlPoint(pos);
		}

		//--------------------------------------------------------------
		bool Canvas::cursorDown(const glm::vec2 & pos)
		{
			this->selectClosestControlPoint(pos);

			if (this->focusedIndex < this->warps.size())
			{
				return this->warps[this->focusedIndex]->handleCursorDown(pos);
			}

			return false;
		}

		//--------------------------------------------------------------
		bool Canvas::cursorDragged(const glm::vec2 & pos)
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
				auto shift = glm::vec2(0.0f);
				if (args.key == OF_KEY_UP)
				{
					shift.y = -step / static_cast<float>(ofGetHeight());
				}
				else if (args.key == OF_KEY_DOWN)
				{
					shift.y = step / static_cast<float>(ofGetHeight());
				}
				else if (args.key == OF_KEY_LEFT)
				{
					shift.x = -step / static_cast<float>(ofGetWidth());
				}
				else
				{
					shift.x = step / static_cast<float>(ofGetWidth());
				}
				warp->moveControlPoint(warp->getSelectedControlPoint(), shift);

				return true;
			}

			return false;
		}

		//--------------------------------------------------------------
		void Canvas::screenResized(ofResizeEventArgs & args)
		{
			this->screenWidth = args.width;
			this->screenHeight = args.height;
			
			if (this->parameters.fillWindow)
			{
				if (this->fboDraw.getWidth() == args.width && this->fboDraw.getHeight() == args.height) return;

				this->fboSettings.width = args.width;
				this->fboSettings.height = args.height;
				this->updateSize();
			}

			for (auto warp : this->warps)
			{
				warp->handleWindowResize(args.width, args.height);
			}
		}
	}
}
