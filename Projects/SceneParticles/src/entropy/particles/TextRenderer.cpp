#include "TextRenderer.h"

namespace{

}

void TextRenderer::setup(float worldSize){
	this->worldSize = worldSize;


	ofTtfSettings fontSettings("fonts/kontrapunkt-bob.light.ttf", 20);
	fontSettings.dpi = 72;
	fontSettings.antialiased = true;
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

	fontSettings.fontSize = 2;
	fonts.emplace_back();
	fonts.back().load(fontSettings);

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
		particleTexts[i]["electron"] = fonts[i].getStringMesh("electron", 0, 0);
		particleTexts[i]["positron"] = fonts[i].getStringMesh("positron", 0, 0);

		particleTexts[i]["quark"] = fonts[i].getStringMesh("quark", 0, 0);
		particleTexts[i]["anti"] = fonts[i].getStringMesh("anti", 0, 0);

		particleTexts[i]["neutron"] = fonts[i].getStringMesh("neutron", 0, 0);
		particleTexts[i]["proton"] = fonts[i].getStringMesh("proton", 0, 0);
	}

	particlePaths.resize(fonts.size());
	for(size_t i = 0; i < fonts.size(); i++){
		ofPath anti;
		anti.setFilled(false);
		anti.setStrokeWidth(1);
		anti.circle(0,0,fonts[i].getStringBoundingBox("x",0,0).height);
		particlePaths[i]["a"] = anti;

		ofPath matter;
		matter.setFilled(true);
		matter.circle(0,0,fonts[i].getStringBoundingBox("x",0,0).height);
		particlePaths[i]["q"] = matter;
	}

}

void TextRenderer::update(nm::ParticleSystem & particles, State state){
	relations.clear();
	auto maxDistance = (relDistance * worldSize) * (relDistance * worldSize);
	switch(state){
		case BARYOGENESIS:
			for(size_t i=0;i<particles.getParticles().size();i++){
				auto & particle = particles.getParticles()[i];
//				std::vector<nm::Particle*> nearList;
//				if(isAntiMatterQuark(particle)){
//					nearList = particles.findNearestThanByType(particle, maxDistance, {nm::Particle::UP_QUARK, nm::Particle::DOWN_QUARK, nm::Particle::UP_DOWN_QUARK});
//				}else if(isMatterQuark(particle)){
//					nearList = particles.findNearestThanByType(particle, maxDistance, {nm::Particle::ANTI_UP_QUARK, nm::Particle::ANTI_DOWN_QUARK});
//				}


			}
		break;
		case STANDARD_MODEL:
		break;
		case NUCLEOSYNTHESIS:
		break;
	}

}

void TextRenderer::draw(nm::ParticleSystem & particles,
						std::vector<nm::Photon> & photons,
						nm::Environment & environment,
						State state,
						std::pair<nm::Particle*, nm::Particle*> lookAt,
						entropy::render::WireframeFillRenderer & renderer,
						ofCamera & cam){
	auto maxScreenDistance = (maxDistance * worldSize) * (maxDistance * worldSize);
	auto scale = environment.getExpansionScalar();

//	glEnable(GL_BLEND);
//	glBlendFunc(GL_ONE, GL_ONE);
	renderer.drawWithDOF(cam, [&](float accumValue, glm::mat4 projection, glm::mat4 modelview){
		ofLoadMatrix(modelview);
		auto maxPDistance = (relDistance * worldSize) * (relDistance * worldSize);
		ofMesh line;
		line.setMode(OF_PRIMITIVE_LINES);
		line.getVertices().resize(2);
		line.getColors().resize(2);
		for(size_t i=0;i<particles.getParticles().size();i++){
			auto & p1  = particles.getParticles()[i];
			auto distance = glm::distance2(cam.getPosition(), p1.pos * scale);
			auto pct = 1-ofClamp(distance / maxScreenDistance, 0, 1);
			for(auto * p2: p1.potentialInteractionPartners){
				//if((p2->getAnnihilationFlag() ^ p1.getAnnihilationFlag()) == 0xFF){
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
					}else */if(pDistance1 < nm::Octree<nm::Particle>::INTERACTION_DISTANCE()){
						auto aniPct = p1.anihilationRatio/environment.getAnnihilationThresh();
						line.getColors()[0] = ofFloatColor(pct*ppct*aniPct, 0, 0, accumValue);
						line.getColors()[1] = ofFloatColor(pct2*ppct*aniPct, 0, 0, accumValue);
						line.getVertices()[1] = glm::lerp(p1.pos, p2->pos, aniPct);
					}else{
						line.getVertices()[1] = p2->pos;
						line.getColors()[0] = ofFloatColor(pct*ppct, accumValue);
						line.getColors()[1] = ofFloatColor(pct2*ppct, accumValue);
					}
					line.draw();
				}
			}
		}
		billboardShaderText.begin();
		auto viewport = ofGetCurrentViewport();
		auto viewportv4 = glm::vec4(viewport.position.xy(), viewport.width, viewport.height);
		billboardShaderText.setUniform4f("viewport", viewportv4);
		billboardShaderText.setUniformMatrix4f("projectionMatrix", projection);
		billboardShaderText.setUniformMatrix4f("modelViewMatrix", modelview);
		billboardShaderText.setUniformMatrix4f("modelViewProjectionMatrix", projection * modelview);
		for(size_t i=0;i<particles.getParticles().size();i++){
			auto & p1  = particles.getParticles()[i];
			if(!p1.alive) continue;
			auto distance = glm::distance2(cam.getPosition(), p1.pos * scale);
			auto pctDistance = ofClamp(distance / maxScreenDistance, 0, 1);
			for(auto * p2: p1.potentialInteractionPartners){
				if(!p2->alive) continue;
				//if((p2->getAnnihilationFlag() ^ p1.getAnnihilationFlag()) == 0xFF){
				if((p1.isAntiMatterQuark() && p2->isMatterQuark()) || (p1.isMatterQuark() && p2->isAntiMatterQuark())){
					auto pctColor = (1-pctDistance) * ambient;
					auto light = std::accumulate(photons.begin(), std::min(photons.begin() + 16, photons.end()), 0.f, [&](float acc, nm::Photon & ph){
						if(ph.alive){
							auto strength = lightStrenght / glm::distance2(p1.pos * scale, ph.pos * scale) * (1 - ofClamp(ph.age / 3.f / environment.systemSpeed, 0, 1));
							return acc + strength;
						}else{
							return acc;
						}
					});
					pctColor += light;
					auto midPoint = (p1.pos + p2->pos) / 2.;
					auto pDistance = glm::distance2(p1.pos * scale, p2->pos * scale);
					auto pDistance1 = glm::distance(p1.pos * scale, p2->pos * scale);
					auto ppct = ofClamp(pDistance / maxPDistance, 0, 1);
					size_t fontSize = ofClamp(size_t(round((particleTexts.size() - 1) * pctDistance)) + 1, 0, particleTexts.size() -1);
					billboardShaderText.setUniform1f("pctColor", pctColor);
					billboardShaderText.setUniform1f("accumValue", accumValue);
					billboardShaderText.setUniform4f("billboard_position", glm::vec4(midPoint, 1.0));
					billboardShaderText.setUniformTexture("tex0", fonts[fontSize].getFontTexture(), 0);
					if(pDistance1 < nm::Octree<nm::Particle>::INTERACTION_DISTANCE()){
						auto aniPct = p1.anihilationRatio/environment.getAnnihilationThresh();
						fonts[fontSize].getStringMesh(ofToString(aniPct), 0,0).draw();
					}else{
						fonts[fontSize].getStringMesh(ofToString(pDistance1), 0,0).draw();
					}
				}
			}
		}
		billboardShaderText.end();
	});


	ofShader & billboardShader = (state == BARYOGENESIS) ? billboardShaderPath : billboardShaderText;
	renderer.drawWithDOF(cam, [&](float accumValue, glm::mat4 projection, glm::mat4 modelview){
		auto i = 0;
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
			if(pctDistance>fulltextDistance){
				switch(p.getType()){
					case nm::Particle::ELECTRON:
						if(state!=BARYOGENESIS) text = "e";
					break;
					case nm::Particle::POSITRON:
					case nm::Particle::PROTON:
						if(state!=BARYOGENESIS) text = "p";
					break;
					case nm::Particle::NEUTRON:
						if(state!=BARYOGENESIS) text = "n";
					break;
					case nm::Particle::ANTI_UP_QUARK:
					case nm::Particle::ANTI_DOWN_QUARK:
						if(state==BARYOGENESIS) text = "a";
					break;
					case nm::Particle::DOWN_QUARK:
					case nm::Particle::UP_QUARK:
					case nm::Particle::UP_DOWN_QUARK:
						if(state==BARYOGENESIS) text = "q";
					break;
				}
			}else{
				switch(p.getType()){
					case nm::Particle::ELECTRON:
						if(state!=BARYOGENESIS) text = "electron";
					break;
					case nm::Particle::POSITRON:
						if(state!=BARYOGENESIS) text = "positron";
					break;
					case nm::Particle::ANTI_UP_QUARK:
					case nm::Particle::ANTI_DOWN_QUARK:
						if(state==BARYOGENESIS) text = "a";
					break;
					case nm::Particle::DOWN_QUARK:
					case nm::Particle::UP_QUARK:
					case nm::Particle::UP_DOWN_QUARK:
						if(state==BARYOGENESIS) text = "q";
					break;
					case nm::Particle::NEUTRON:
						if(state!=BARYOGENESIS) text = "neutron";
					break;
					case nm::Particle::PROTON:
						if(state!=BARYOGENESIS) text = "proton";
					break;
					break;
				}
			}
			if(text!=""){
				size_t fontSize = size_t(round((particleTexts.size() - 1) * pctDistance));
				if(state==BARYOGENESIS) {
					particlePaths[fontSize][text].draw();
				}else{
					billboardShader.setUniformTexture("tex0", fonts[fontSize].getFontTexture(), 0);
					particleTexts[fontSize][text].draw();
				}
			}
			//font.getStringMesh(text, 0, 0).draw();
			//billboard(font, text, cam.getProjectionMatrix(), modelview, p);
		}


		for(auto & p: photons){
			if(!p.alive) continue;
			auto distance = glm::distance2(cam.getPosition(), p.pos * scale);
			auto pctDistance = ofClamp(distance / maxScreenDistance, 0, 1);
			auto pctColor = (1-pctDistance) * photonsStrenght * (1 - ofClamp(p.age/3/environment.systemSpeed, 0, 1));
			billboardShader.setUniform1f("pctColor", pctColor);
			billboardShader.setUniform1f("accumValue", accumValue);
			billboardShader.setUniform4f("billboard_position", glm::vec4(p.pos * scale, 1.0));
			auto concentrics = int(ofMap(p.age / environment.systemSpeed, 0, 1.5, 0, 3)) % 3 + 1;
			int fontSize = size_t(round((particleTexts.size() - 1) * pctDistance));
			fontSize = ofClamp(fontSize-2, 2, particlePaths.size()-1);
			for(int i = 0; i < concentrics; i++){
				particlePaths[fontSize]["a"].draw();
				fontSize -= 2;
				fontSize %= particlePaths.size();
			}
		}
		billboardShader.end();
	});
}
