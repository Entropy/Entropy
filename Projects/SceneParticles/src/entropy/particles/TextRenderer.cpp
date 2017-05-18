#include "TextRenderer.h"

namespace{
	bool isMatterQuark(const nm::Particle & p){
		switch(p.getType()){
			case nm::Particle::UP_QUARK:
			case nm::Particle::DOWN_QUARK:
			case nm::Particle::UP_DOWN_QUARK:
				return true;
			case nm::Particle::ANTI_UP_QUARK:
			case nm::Particle::ANTI_DOWN_QUARK:
			default:
				return false;
		}
	}

	bool isAntiMatterQuark(const nm::Particle & p){
		switch(p.getType()){
			case nm::Particle::ANTI_UP_QUARK:
			case nm::Particle::ANTI_DOWN_QUARK:
				return true;
			case nm::Particle::UP_QUARK:
			case nm::Particle::DOWN_QUARK:
			case nm::Particle::UP_DOWN_QUARK:
			default:
				return false;
		}
	}

	bool isQuark(const nm::Particle & p){
		switch(p.getType()){
			case nm::Particle::ANTI_UP_QUARK:
			case nm::Particle::ANTI_DOWN_QUARK:
			case nm::Particle::UP_QUARK:
			case nm::Particle::DOWN_QUARK:
			case nm::Particle::UP_DOWN_QUARK:
				return true;
			default:
				return false;
		}
	}
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

	billboardShader.load("shaders/billboard.vert.glsl", "shaders/billboard.frag.glsl");

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

}

void TextRenderer::update(nm::ParticleSystem & particles, State state){
	relations.clear();
	auto maxDistance = (relDistance * worldSize) * (relDistance * worldSize);
	switch(state){
		case BARYOGENESIS:
			for(size_t i=0;i<particles.getParticles().size();i++){
				auto & particle = particles.getParticles()[i];
				std::vector<nm::Particle*> nearList;
				if(isAntiMatterQuark(particle)){
					nearList = particles.findNearestThanByType(particle, maxDistance, {nm::Particle::UP_QUARK, nm::Particle::DOWN_QUARK, nm::Particle::UP_DOWN_QUARK});
				}else if(isMatterQuark(particle)){
					nearList = particles.findNearestThanByType(particle, maxDistance, {nm::Particle::ANTI_UP_QUARK, nm::Particle::ANTI_DOWN_QUARK});
				}
				if(!nearList.empty()){
					relations.push_back(std::make_pair(i, nearList));
				}
			}
		break;
		case STANDARD_MODEL:
		break;
		case NUCLEOSYNTHESIS:
		break;
	}

}

void TextRenderer::draw(nm::ParticleSystem & particles, nm::Environment & environment, State state, entropy::render::WireframeFillRenderer & renderer, ofCamera & cam){
	auto maxScreenDistance = (maxDistance * worldSize) * (maxDistance * worldSize);
	auto scale = environment.getExpansionScalar();

//	glEnable(GL_BLEND);
//	glBlendFunc(GL_ONE, GL_ONE);
	renderer.drawWithDOF(cam, [&](float accumValue, glm::mat4 projection, glm::mat4 modelview){
		ofLoadMatrix(modelview);
		auto maxPDistance = (relDistance * worldSize) * (relDistance * worldSize);
		for(auto & r: relations){
			auto & p1 = particles.getParticles()[r.first];
			auto & nearp = r.second;
			for(auto * p2: nearp){
				auto distance = glm::distance2(cam.getPosition(), p1 * scale);
				auto pDistance = glm::distance2(p1.xyz() * scale, p2->xyz() * scale);
				auto pct = 1-ofClamp(distance / maxScreenDistance, 0, 1);
				auto ppct = 1-ofClamp(pDistance / maxPDistance, 0, 1);
				ofSetColor(pct*ppct*255, accumValue*ppct*255);
				ofDrawLine(p1,*p2);

			}
		}
		billboardShader.begin();
		auto viewport = ofGetCurrentViewport();
		auto viewportv4 = glm::vec4(viewport.position.xy(), viewport.width, viewport.height);
		billboardShader.setUniform4f("viewport", viewportv4);
		billboardShader.setUniformMatrix4f("projectionMatrix", projection);
		billboardShader.setUniformMatrix4f("modelViewMatrix", modelview);
		billboardShader.setUniformMatrix4f("modelViewProjectionMatrix", projection * modelview);
		for(auto & r: relations){
			auto & p1 = particles.getParticles()[r.first];
			auto & nearp = r.second;
			auto distance = glm::distance2(cam.getPosition(), p1 * scale);
			auto pctDistance = ofClamp(distance / maxScreenDistance, 0, 1);
			auto pctColor = 1-pctDistance;
			for(auto * p2: nearp){
				auto midPoint = (p1 + *p2) / 2.;
				auto pDistance = glm::distance2(p1.xyz() * scale, p2->xyz() * scale);
				auto ppct = ofClamp(pDistance / maxPDistance, 0, 1);
				size_t fontSize = size_t(round((particleTexts.size() - 1) * pctDistance));
				billboardShader.setUniform1f("pctColor", pctColor);
				billboardShader.setUniform1f("accumValue", accumValue);
				billboardShader.setUniform4f("billboard_position", glm::vec4(midPoint, 1.0));
				billboardShader.setUniformTexture("tex0", fonts[fontSize].getFontTexture(), 0);
				fonts[fontSize].getStringMesh(ofToString(ppct), 0,0).draw();
			}
		}
		billboardShader.end();
	});

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
			auto distance = glm::distance2(cam.getPosition(), p * scale);
			auto pctDistance = ofClamp(distance / maxScreenDistance, 0, 1);
			auto pctColor = 1-pctDistance;
			billboardShader.setUniform1f("pctColor", pctColor);
			billboardShader.setUniform1f("accumValue", accumValue);
			billboardShader.setUniform4f("billboard_position", glm::vec4(p * scale, 1.0));
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
				billboardShader.setUniformTexture("tex0", fonts[fontSize].getFontTexture(), 0);
				particleTexts[fontSize][text].draw();
				fonts[fontSize].getStringMesh(ofToString(p.getAnnihilationFlag()), 0,0).draw();
			}
			//font.getStringMesh(text, 0, 0).draw();
			//billboard(font, text, cam.getProjectionMatrix(), modelview, p);
		}
		billboardShader.end();
	});
}
