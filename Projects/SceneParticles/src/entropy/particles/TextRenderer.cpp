#include "TextRenderer.h"

namespace{

}

void TextRenderer::setup(float worldSize, int width, int height, Screen screen){
	this->worldSize = worldSize;
	this->width = width;
	this->height = height;
	this->screen = screen;


	ofTtfSettings fontSettings("fonts/kontrapunkt-bob.light.ttf", 20);
	fontSettings.dpi = 72;
	fontSettings.antialiased = true;
	fonts.emplace_back();
	fonts.back().load(fontSettings);

	if(screen == FrontScreen){
		fontSettings.fontSize = 36;
		fonts.emplace_back();
		fonts.back().load(fontSettings);

		fontSettings.fontSize = 34;
		fonts.emplace_back();
		fonts.back().load(fontSettings);

		fontSettings.fontSize = 32;
		fonts.emplace_back();
		fonts.back().load(fontSettings);

		fontSettings.fontSize = 30;
		fonts.emplace_back();
		fonts.back().load(fontSettings);

		fontSettings.fontSize = 28;
		fonts.emplace_back();
		fonts.back().load(fontSettings);

		fontSettings.fontSize = 26;
		fonts.emplace_back();
		fonts.back().load(fontSettings);

		fontSettings.fontSize = 24;
		fonts.emplace_back();
		fonts.back().load(fontSettings);

		fontSettings.fontSize = 22;
		fonts.emplace_back();
		fonts.back().load(fontSettings);

		fontSettings.fontSize = 20;
		fonts.emplace_back();
		fonts.back().load(fontSettings);

		fontSettings.fontSize = 18;
		fonts.emplace_back();
		fonts.back().load(fontSettings);

		fontSettings.fontSize = 16;
		fonts.emplace_back();
		fonts.back().load(fontSettings);

		fontSettings.fontSize = 14;
		fonts.emplace_back();
		fonts.back().load(fontSettings);

		fontSettings.fontSize = 12;
		fonts.emplace_back();
		fonts.back().load(fontSettings);

		fontSettings.fontSize = 10;
		fonts.emplace_back();
		fonts.back().load(fontSettings);

		fontSettings.fontSize = 8;
		fonts.emplace_back();
		fonts.back().load(fontSettings);

		fontSettings.fontSize = 6;
		fonts.emplace_back();
		fonts.back().load(fontSettings);
	}else{
		fontSettings.fontSize = 20;
		fonts.emplace_back();
		fonts.back().load(fontSettings);

		fontSettings.fontSize = 18;
		fonts.emplace_back();
		fonts.back().load(fontSettings);

		fontSettings.fontSize = 16;
		fonts.emplace_back();
		fonts.back().load(fontSettings);

		fontSettings.fontSize = 14;
		fonts.emplace_back();
		fonts.back().load(fontSettings);

		fontSettings.fontSize = 12;
		fonts.emplace_back();
		fonts.back().load(fontSettings);

		fontSettings.fontSize = 10;
		fonts.emplace_back();
		fonts.back().load(fontSettings);

		fontSettings.fontSize = 8;
		fonts.emplace_back();
		fonts.back().load(fontSettings);

		fontSettings.fontSize = 6;
		fonts.emplace_back();
		fonts.back().load(fontSettings);

		fontSettings.fontSize = 4;
		fonts.emplace_back();
		fonts.back().load(fontSettings);
	}


	ofShader::Settings shaderSettings;
	shaderSettings.shaderFiles[GL_VERTEX_SHADER] = "shaders/billboard.vert.glsl";
	shaderSettings.shaderFiles[GL_FRAGMENT_SHADER] = "shaders/billboard.frag.glsl";
	shaderSettings.intDefines["HAS_TEXTURE"] = 1;
	billboardShaderText.setup(shaderSettings);
	shaderSettings.intDefines["HAS_TEXTURE"] = 0;
	billboardShaderPath.setup(shaderSettings);

	particleTexts.resize(fonts.size());
	for(size_t i = 0; i < fonts.size(); i++){
		particleTexts[i]["e"] = fonts[i].getStringMesh("e", 0, 0);
		particleTexts[i]["p"] = fonts[i].getStringMesh("p", 0, 0);
		particleTexts[i]["q"] = fonts[i].getStringMesh("q", 0, 0);
		particleTexts[i]["a"] = fonts[i].getStringMesh("a", 0, 0);
		particleTexts[i]["n"] = fonts[i].getStringMesh("n", 0, 0);
		particleTexts[i]["uq"] = fonts[i].getStringMesh("uq", 0, 0);
		particleTexts[i]["dq"] = fonts[i].getStringMesh("dq", 0, 0);


		particleTexts[i]["electron"] = fonts[i].getStringMesh("electron", 0, 0);
		particleTexts[i]["positron"] = fonts[i].getStringMesh("positron", 0, 0);

		particleTexts[i]["quark"] = fonts[i].getStringMesh("quark", 0, 0);
		particleTexts[i]["anti"] = fonts[i].getStringMesh("anti", 0, 0);
		particleTexts[i]["up quark"] = fonts[i].getStringMesh("up\nquark", 0, 0);
		particleTexts[i]["down quark"] = fonts[i].getStringMesh("down\nquark", 0, 0);

		particleTexts[i]["neutron"] = fonts[i].getStringMesh("neutron", 0, 0);
		particleTexts[i]["proton"] = fonts[i].getStringMesh("proton", 0, 0);
	}

	particlePaths.resize(fonts.size());
	for(size_t i = 0; i < fonts.size(); i++){
		ofPath anti;
		anti.setFilled(true);
		anti.circle(0,0,fonts[i].getStringBoundingBox("x",0,0).height + 1);
		anti.circle(0,0,fonts[i].getStringBoundingBox("x",0,0).height - 2);
		particlePaths[i]["a"] = anti;

		ofPath matter;
		matter.setFilled(true);
		matter.circle(0,0,fonts[i].getStringBoundingBox("x",0,0).height);
		particlePaths[i]["q"] = matter;
	}

	if(screen == FrontScreen){
		fboLines.allocate(width/3, height/3, GL_RGBA, 16);
	}

}

void TextRenderer::update(nm::ParticleSystem & particles, nm::Environment & environment){
	auto maxDistance = (relDistance * worldSize) * (relDistance * worldSize);
	switch(environment.state){
		case nm::Environment::BARYOGENESIS:
		break;
		case nm::Environment::STANDARD_MODEL:
		break;
		case nm::Environment::NUCLEOSYNTHESIS:
		break;
	}

}


void TextRenderer::renderLines(nm::ParticleSystem & particles,
				 std::vector<nm::Photon> & photons,
				 nm::Environment & environment,
				 entropy::render::WireframeFillRenderer & renderer,
				 ofCamera & cam){

	auto maxScreenDistance = (maxDistance * worldSize) * (maxDistance * worldSize);
	auto scale = environment.getExpansionScalar();
	renderer.drawWithDOF(cam, [&](float accumValue, glm::mat4 projection, glm::mat4 modelview){
		if(screen == FrontScreen){
			fboLines.begin();
		}
		ofClear(0,0);
		cam.begin();
		ofLoadMatrix(modelview);
		auto maxPDistance = (relDistance * worldSize) * (relDistance * worldSize);

		// Particle <-> particle relations
		switch(environment.state.get()){
			case nm::Environment::BARYOGENESIS:{
				ofMesh line;
				line.setMode(OF_PRIMITIVE_LINES);
				line.getVertices().resize(2);
				line.getColors().resize(2);
				for(auto & p1: particles.getParticles()){
					//if(!p1.alive) continue;
					auto distance = glm::distance2(cam.getPosition(), p1.pos * scale);
					auto pct = 1-ofClamp(distance / maxScreenDistance, 0, 1);
					for(auto * p2: p1.potentialInteractionPartners){
						if(p1.id < p2->id && ((p1.isAntiMatterQuark() && p2->isMatterQuark()) || (p1.isMatterQuark() && p2->isAntiMatterQuark()))){
							auto pDistance = glm::distance2(p1.pos * scale, p2->pos * scale);
							auto pDistance1 = glm::distance(p1.pos * scale, p2->pos * scale);
							auto ppct = (1-ofClamp(pDistance / maxPDistance, 0, 1)) * ambient;
							auto light = std::accumulate(photons.begin(), std::min(photons.begin() + 16, photons.end()), 0.f, [&](float acc, nm::Photon & ph){
								if(ph.alive){
									auto strength = lightStrenght / glm::distance2(p1.pos * scale, ph.pos * scale) * (1 - ofClamp(ph.age / 3.f / environment.systemSpeed, 0, 1));
									return acc + strength;
								}else{
									return acc;
								}
							});
							ppct += light;
							auto distancep2 = glm::distance2(cam.getPosition(), p2->pos * scale);
							auto pct2 = (1-ofClamp(distancep2 / maxScreenDistance, 0, 1)) * ambient + light;
							line.getVertices()[0] = p1.pos;
							/*if(lookAt.first && lookAt.second && ((p1.id == lookAt.first->id && p2->id == lookAt.second->id) || (p1.id == lookAt.second->id && p2->id == lookAt.first->id))){
								line.getColors()[0] = ofFloatColor(0, pct*ppct, 0, accumValue);
								line.getColors()[1] = ofFloatColor(0, pct2*ppct, 0, accumValue);
								line.getVertices()[1] = p2->pos;
								line.draw();
							}else */if(pDistance1 < nm::Octree<nm::Particle>::INTERACTION_DISTANCE()){
								auto aniPct = p1.anihilationRatio/environment.getAnnihilationThresh();
								line.getColors()[0] = ofFloatColor(pct*ppct*aniPct, 0, 0, accumValue);
								line.getColors()[1] = ofFloatColor(pct2*ppct*aniPct, 0, 0, accumValue);
								line.getVertices()[1] = glm::lerp(p1.pos, p2->pos, aniPct);

//								auto white0 = ofFloatColor(pct*ppct*aniPct, 0, 0, accumValue);
//								auto red0 = ofFloatColor(pct*ppct*aniPct, 0, 0, accumValue);
//								line.getColors()[0] = white0.lerp(red0, aniPct);

//								auto white1 = ofFloatColor(pct2*ppct*aniPct, 0, 0, accumValue);
//								auto red1 = ofFloatColor(pct2*ppct*aniPct, 0, 0, accumValue);

//								line.getColors()[1] = white1.lerp(red1, aniPct);
								line.getVertices()[1] = p2->pos;
								line.draw();
							}else{
								line.getVertices()[1] = p2->pos;
								line.getColors()[0] = ofFloatColor(pct*ppct, accumValue);
								line.getColors()[1] = ofFloatColor(pct2*ppct, accumValue);
								line.draw();
							}
						}
					}
				}
			}break;
			case nm::Environment::STANDARD_MODEL:
			break;
			case nm::Environment::NUCLEOSYNTHESIS:{
				ofMesh line;
				line.setMode(OF_PRIMITIVE_LINES);
				line.getVertices().resize(6);
				line.getColors().resize(6);//.assign(6, ofFloatColor(1));
				for(auto & p1: particles.getParticles()){
					auto distancep1 = glm::distance2(cam.getPosition(), p1.pos * scale);
					auto * p2 = p1.fusionPartners.first;
					auto * p3 = p1.fusionPartners.second;
					if(p2 && p3){
						auto midPoint = (p1.pos + p2->pos + p3->pos) / 3.;
						float ppct = ambient;
						auto light = std::accumulate(photons.begin(), std::min(photons.begin() + 16, photons.end()), 0.f, [&](float acc, nm::Photon & ph){
							if(ph.alive){
								auto strength = lightStrenght / glm::distance2(midPoint * scale, ph.pos * scale) * (1 - ofClamp(ph.age / 3.f / environment.systemSpeed, 0, 1));
								return acc + strength;
							}else{
								return acc;
							}
						});
						ppct += light;
						auto distancep2 = glm::distance2(cam.getPosition(), p2->pos * scale);
						auto distancep3 = glm::distance2(cam.getPosition(), p3->pos * scale);
						auto pct1 = (1-ofClamp(distancep1 / maxScreenDistance, 0, 1)) * ambient + light;
						auto pct2 = (1-ofClamp(distancep2 / maxScreenDistance, 0, 1)) * ambient + light;
						auto pct3 = (1-ofClamp(distancep3 / maxScreenDistance, 0, 1)) * ambient + light;
						line.getVertices()[0] = p1.pos;
						line.getVertices()[1] = midPoint;
						/*if(lookAt.first && lookAt.second && ((p1.id == lookAt.first->id && p2->id == lookAt.second->id) || (p1.id == lookAt.second->id && p2->id == lookAt.first->id))){
							line.getColors()[0] = ofFloatColor(0, pct*ppct, 0, accumValue);
							line.getColors()[1] = ofFloatColor(0, pct2*ppct, 0, accumValue);
							line.getVertices()[1] = p2->pos;
							line.draw();
						}else if(pDistance1 < nm::Octree<nm::Particle>::INTERACTION_DISTANCE()){*/
							auto fusPct = p1.fusionRatio/environment.getFusionThresh();
							line.getColors()[0] = ofFloatColor(pct1*ppct, accumValue)
													.lerp(ofFloatColor(pct1*ppct, 0, 0, accumValue), fusPct);
							line.getColors()[1] = ofFloatColor(pct1*ppct, accumValue)
													.lerp(ofFloatColor(pct1*ppct, 0, 0, accumValue), fusPct);
							line.getColors()[2] = ofFloatColor(pct2*ppct, accumValue)
													.lerp(ofFloatColor(pct2*ppct, 0, 0, accumValue), fusPct);
							line.getColors()[3] = ofFloatColor(pct2*ppct, accumValue)
													.lerp(ofFloatColor(pct2*ppct, 0, 0, accumValue), fusPct);
							line.getColors()[4] = ofFloatColor(pct3*ppct, accumValue)
													.lerp(ofFloatColor(pct3*ppct, 0, 0, accumValue), fusPct);
							line.getColors()[5] = ofFloatColor(pct3*ppct, accumValue)
													.lerp(ofFloatColor(pct3*ppct, 0, 0, accumValue), fusPct);
							line.getVertices()[2] = p2->pos;
							line.getVertices()[3] = midPoint;
							line.getVertices()[4] = p3->pos;
							line.getVertices()[5] = midPoint;
							line.draw();
//						}else{
//							line.getVertices()[1] = p2->pos;
//							line.getColors()[0] = ofFloatColor(pct*ppct, accumValue);
//							line.getColors()[1] = ofFloatColor(pct2*ppct, accumValue);
//							line.draw();
//						}
					}
				}
			}
			break;
		}
		cam.end();
		if(screen == FrontScreen){
			fboLines.end();
		}
	});

	if(screen == FrontScreen){
		fboLines.draw(0,height,width,-height);
	}
}


void TextRenderer::draw(nm::ParticleSystem & particles,
						std::vector<nm::Photon> & photons,
						nm::Environment & environment,
						std::pair<nm::Particle*, nm::Particle*> lookAt,
						entropy::render::WireframeFillRenderer & renderer,
						ofCamera & cam){
	auto maxScreenDistance = (maxDistance * worldSize) * (maxDistance * worldSize);
	auto scale = environment.getExpansionScalar();

//	glEnable(GL_BLEND);
//	glBlendFunc(GL_ONE, GL_ONE);


	renderer.drawWithDOF(cam, [&](float accumValue, glm::mat4 projection, glm::mat4 modelview){
		ofLoadMatrix(modelview);

		// Relation info text, currently distance but doesn't make a lot of sense
		billboardShaderText.begin();
		auto viewport = ofGetCurrentViewport();
		auto viewportv4 = glm::vec4(viewport.position.xy(), viewport.width, viewport.height);
		billboardShaderText.setUniform4f("viewport", viewportv4);
		billboardShaderText.setUniformMatrix4f("projectionMatrix", projection);
		billboardShaderText.setUniformMatrix4f("modelViewMatrix", modelview);
		billboardShaderText.setUniformMatrix4f("modelViewProjectionMatrix", projection * modelview);
		for(auto & p1: particles.getParticles()){
			if(!p1.alive) continue;
			auto distance = glm::distance2(cam.getPosition(), p1.pos * scale);
			auto pctDistance = ofClamp(distance / maxScreenDistance, 0, 1);
			for(auto * p2: p1.potentialInteractionPartners){
				if(!p2->alive) continue;
				//if((p2->getAnnihilationFlag() ^ p1.getAnnihilationFlag()) == 0xFF){

				switch(environment.state.get()){
					case nm::Environment::BARYOGENESIS:
//						if((p1.isAntiMatterQuark() && p2->isMatterQuark()) || (p1.isMatterQuark() && p2->isAntiMatterQuark())){
//							auto pctColor = (1-pctDistance) * ambient;
//							auto light = std::accumulate(photons.begin(), std::min(photons.begin() + 16, photons.end()), 0.f, [&](float acc, nm::Photon & ph){
//								if(ph.alive){
//									auto strength = lightStrenght / glm::distance2(p1.pos * scale, ph.pos * scale) * (1 - ofClamp(ph.age / 3.f / environment.systemSpeed, 0, 1));
//									return acc + strength;
//								}else{
//									return acc;
//								}
//							});
//							pctColor += light;
//							auto midPoint = (p1.pos + p2->pos) / 2.;
//							auto pDistance = glm::distance2(p1.pos * scale, p2->pos * scale);
//							auto pDistance1 = glm::distance(p1.pos * scale, p2->pos * scale);
//							auto ppct = ofClamp(pDistance / maxPDistance, 0, 1);
//							size_t fontSize = ofClamp(size_t(round((particleTexts.size() - 1) * pctDistance)) + 1, 0, particleTexts.size() -1);
//							billboardShaderText.setUniform1f("pctColor", pctColor);
//							billboardShaderText.setUniform1f("accumValue", accumValue);
//							billboardShaderText.setUniform4f("billboard_position", glm::vec4(midPoint, 1.0));
//							billboardShaderText.setUniformTexture("tex0", fonts[fontSize].getFontTexture(), 0);
//							if(pDistance1 < nm::Octree<nm::Particle>::INTERACTION_DISTANCE()){
//								auto aniPct = p1.anihilationRatio/environment.getAnnihilationThresh();
//								fonts[fontSize].getStringMesh(ofToString(aniPct), 0,0).draw();
//							}else{
//								fonts[fontSize].getStringMesh(ofToString(pDistance1), 0,0).draw();
//							}
//						}
					break;
					case nm::Environment::STANDARD_MODEL:
//						if((p2->getFusion1Flag() ^ p1.getFusion1Flag()) == 0xFF ||
//						   (p2->getFusion2Flag() ^ p1.getFusion2Flag()) == 0xFF){
//							auto pctColor = (1-pctDistance) * ambient;
//							auto light = std::accumulate(photons.begin(), std::min(photons.begin() + 16, photons.end()), 0.f, [&](float acc, nm::Photon & ph){
//								if(ph.alive){
//									auto strength = lightStrenght / glm::distance2(p1.pos * scale, ph.pos * scale) * (1 - ofClamp(ph.age / 3.f / environment.systemSpeed, 0, 1));
//									return acc + strength;
//								}else{
//									return acc;
//								}
//							});
//							pctColor += light;
//							auto midPoint = (p1.pos + p2->pos) / 2.;
//							auto pDistance = glm::distance2(p1.pos * scale, p2->pos * scale);
//							auto pDistance1 = glm::distance(p1.pos * scale, p2->pos * scale);
//							auto ppct = ofClamp(pDistance / maxPDistance, 0, 1);
//							size_t fontSize = ofClamp(size_t(round((particleTexts.size() - 1) * pctDistance)) + 1, 0, particleTexts.size() -1);
//							billboardShaderText.setUniform1f("pctColor", pctColor);
//							billboardShaderText.setUniform1f("accumValue", accumValue);
//							billboardShaderText.setUniform4f("billboard_position", glm::vec4(midPoint, 1.0));
//							billboardShaderText.setUniformTexture("tex0", fonts[fontSize].getFontTexture(), 0);
//							if(pDistance1 < nm::Octree<nm::Particle>::INTERACTION_DISTANCE()){
//								fonts[fontSize].getStringMesh(ofToString(p1.fusionRatio), 0,0).draw();
//							}else{
//								fonts[fontSize].getStringMesh(ofToString(pDistance1), 0,0).draw();
//							}
//						}
					break;
					case nm::Environment::NUCLEOSYNTHESIS:
					break;
				}
			}
		}
		billboardShaderText.end();
	});


	// Render actual particles
	ofShader & billboardShader = (environment.state == nm::Environment::BARYOGENESIS || environment.state == nm::Environment::STANDARD_MODEL) ? billboardShaderPath : billboardShaderText;
	renderer.drawWithDOF(cam, [&](float accumValue, glm::mat4 projection, glm::mat4 modelview){
		billboardShader.begin();
		auto viewport = ofGetCurrentViewport();
		auto viewportv4 = glm::vec4(viewport.position.xy(), viewport.width, viewport.height);
		billboardShader.setUniform4f("viewport", viewportv4);
		billboardShader.setUniformMatrix4f("projectionMatrix", projection);
		billboardShader.setUniformMatrix4f("modelViewMatrix", modelview);
		billboardShader.setUniformMatrix4f("modelViewProjectionMatrix", projection * modelview);
		for(auto & p: particles.getParticles()){
			if(!p.alive) continue;
			auto distance = glm::distance2(cam.getPosition(), p.pos * scale);
			auto pctDistance = ofClamp(distance / maxScreenDistance, 0, 1);
			auto pctColor = (1-pctDistance) * ambient;
			auto light = std::accumulate(photons.begin(), std::min(photons.begin() + 16, photons.end()), 0.f, [&](float acc, nm::Photon & ph){
				if(ph.alive){
					auto strength = lightStrenght / glm::distance2(p.pos * scale, ph.pos * scale) * (1 - ofClamp(ph.age / 3.f / environment.systemSpeed, 0, 1));
					return acc + strength;
				}else{
					return acc;
				}
			});
			pctColor += light;
			billboardShader.setUniform1f("pctColor", pctColor);
			billboardShader.setUniform1f("accumValue", accumValue);
			billboardShader.setUniform4f("billboard_position", glm::vec4(p.pos * scale, 1.0));
			std::string text = "";
			switch(environment.state){
				case nm::Environment::BARYOGENESIS:
				case nm::Environment::STANDARD_MODEL:
					switch(p.getType()){
						case nm::Particle::ANTI_UP_QUARK:
						case nm::Particle::ANTI_DOWN_QUARK:
							if(environment.state==nm::Environment::BARYOGENESIS) text = "a";
						break;
						case nm::Particle::DOWN_QUARK:
						case nm::Particle::UP_QUARK:
							text = "q";
						break;
						case nm::Particle::ELECTRON:
						case nm::Particle::POSITRON:
						case nm::Particle::PROTON:
						case nm::Particle::NEUTRON:
							// wont render here
						break;
					}
					if(text!=""){
						size_t fontSize = size_t(round((particleTexts.size() - 1) * pctDistance));
						particlePaths[fontSize][text].draw();
					}
				break;
					switch(p.getType()){
						case nm::Particle::DOWN_QUARK:
						case nm::Particle::UP_QUARK:{
							text = "q";
							size_t fontSize = size_t(round((particleTexts.size() - 1) * pctDistance));
							particlePaths[fontSize][text].draw();
						}
						break;
						case nm::Particle::ELECTRON:
						case nm::Particle::ANTI_UP_QUARK:
						case nm::Particle::ANTI_DOWN_QUARK:
						case nm::Particle::POSITRON:
						case nm::Particle::PROTON:
						case nm::Particle::NEUTRON:
							// wont render here
						break;
					}
					if(text!=""){
						size_t fontSize = size_t(round((particleTexts.size() - 1) * pctDistance));
						particlePaths[fontSize][text].draw();
					}
				break;
				case nm::Environment::NUCLEOSYNTHESIS:
//					if(pctDistance > fulltextDistance && p.age > 1){
//						switch(p.getType()){
//							case nm::Particle::ELECTRON:
//								text = "e";
//							break;
//							case nm::Particle::POSITRON:
//							break;
//							case nm::Particle::PROTON:
//								text = "p";
//							break;
//							case nm::Particle::NEUTRON:
//								text = "n";
//							break;
//							case nm::Particle::ANTI_UP_QUARK:
//							case nm::Particle::ANTI_DOWN_QUARK:
//								//text = "a"; // TODO: show this??
//							break;
//							case nm::Particle::DOWN_QUARK:
//								text = "dq"; // TODO: show this??
//							break;
//							case nm::Particle::UP_QUARK:
//								text = "uq";
//							break;
//						}
//					}else{
						switch(p.getType()){
							case nm::Particle::ELECTRON:
								text = "electron";
							break;
							case nm::Particle::POSITRON:
								//text = "positron";
							break;
							case nm::Particle::ANTI_UP_QUARK:
							case nm::Particle::ANTI_DOWN_QUARK:
								//text = "anti quark"; // TODO: show this??
							break;
							case nm::Particle::DOWN_QUARK:
								text = "down quark"; // TODO: show this??
							break;
							case nm::Particle::UP_QUARK:
								text = "up quark"; // TODO: show this??
							break;
							case nm::Particle::NEUTRON:
								text = "neutron";
							break;
							case nm::Particle::PROTON:
								text = "proton";
							break;
						}
						if(text!=""){
							size_t fontSize = size_t(round((particleTexts.size() - 2) * pctDistance));
							if(text == "neutron" || text == "proton"){
								fontSize += 1;
							}
							billboardShader.setUniformTexture("tex0", fonts[fontSize].getFontTexture(), 0);
							particleTexts[fontSize][text].draw();
						}
//					}
				break;
			}

		}
		billboardShader.end();

		if(environment.state == nm::Environment::STANDARD_MODEL){
			billboardShaderText.begin();
			auto viewport = ofGetCurrentViewport();
			auto viewportv4 = glm::vec4(viewport.position.xy(), viewport.width, viewport.height);
			billboardShaderText.setUniform4f("viewport", viewportv4);
			billboardShaderText.setUniformMatrix4f("projectionMatrix", projection);
			billboardShaderText.setUniformMatrix4f("modelViewMatrix", modelview);
			billboardShaderText.setUniformMatrix4f("modelViewProjectionMatrix", projection * modelview);
			string text = "e";
			for(auto & p: particles.getParticles()){
				if(!p.alive) continue;
				if(p.getType()!=nm::Particle::ELECTRON) continue;
				auto distance = glm::distance2(cam.getPosition(), p.pos * scale);
				auto pctDistance = ofClamp(distance / maxScreenDistance, 0, 1);
				auto pctColor = (1-pctDistance) * ambient;
				auto light = std::accumulate(photons.begin(), std::min(photons.begin() + 16, photons.end()), 0.f, [&](float acc, nm::Photon & ph){
					if(ph.alive){
						auto strength = lightStrenght / glm::distance2(p.pos * scale, ph.pos * scale) * (1 - ofClamp(ph.age / 3.f / environment.systemSpeed, 0, 1));
						return acc + strength;
					}else{
						return acc;
					}
				});
				pctColor += light;
				billboardShaderText.setUniform1f("pctColor", pctColor);
				billboardShaderText.setUniform1f("accumValue", accumValue);
				billboardShaderText.setUniform4f("billboard_position", glm::vec4(p.pos * scale, 1.0));

				size_t fontSize = size_t(round((particleTexts.size() - 1) * pctDistance));
				billboardShader.setUniformTexture("tex0", fonts[fontSize].getFontTexture(), 0);
				particleTexts[fontSize][text].draw();
			}
			billboardShaderText.end();
		}

		// Phtons
		billboardShaderPath.begin();
		for(auto & p: photons){
			if(!p.alive) continue;
			auto distance = glm::distance2(cam.getPosition(), p.pos * scale);
			auto pctDistance = ofClamp(distance / maxScreenDistance, 0, 1);
			auto pctColor = (1-pctDistance) * photonsStrenght * (1 - ofClamp(p.age/nm::Photons::LIVE()/environment.systemSpeed, 0, 1));
			billboardShaderPath.setUniform1f("pctColor", pctColor);
			billboardShaderPath.setUniform1f("accumValue", accumValue);
			billboardShaderPath.setUniform4f("billboard_position", glm::vec4(p.pos * scale, 1.0));
			auto concentrics = int(ofMap(p.age / environment.systemSpeed, 0, 0.5, 0, 3)) % 3 + 1;
			int fontSize = size_t(round((particleTexts.size() - 1) * pctDistance));
			fontSize = ofClamp(fontSize-2, 2, particlePaths.size()-1);
			for(int i = 0; i < concentrics; i++){
				particlePaths[fontSize]["a"].draw();
				fontSize -= 2;
				fontSize %= particlePaths.size();
			}
		}
		billboardShaderPath.end();
	});
}
