#include "CmbSceneGL3D.h"

namespace ent
{
	//--------------------------------------------------------------
	void CmbSceneGL3D::setup()
	{
		CmbScene::setup();
		
		// Allocate the textures and buffers.
		for (int i = 0; i < 3; ++i) {
			m_textures[i].allocate(m_dimensions.x, m_dimensions.y, m_dimensions.z, GL_RGBA32F);

			m_fbos[i].allocate();
			m_fbos[i].attachTexture(m_textures[i], 0);
			m_fbos[i].begin();
			{
				ofClear(0, 0);
			}
			m_fbos[i].end();
			m_fbos[i].checkStatus();
		}

		// Build a mesh to render a quad.
		ofDefaultVec3 origin = ofDefaultVec3(0.0, ofGetHeight() - m_dimensions.y);

		m_mesh.setMode(OF_PRIMITIVE_TRIANGLES);
		m_mesh.addVertex(origin + ofDefaultVec3(0.0, 0.0, 0.0));
		m_mesh.addVertex(origin + ofDefaultVec3(m_dimensions.x, 0.0, 0.0));
		m_mesh.addVertex(origin + ofDefaultVec3(0.0, m_dimensions.y, 0.0));

		m_mesh.addTexCoord(ofVec2f(0.0, 0.0));
		m_mesh.addTexCoord(ofVec2f(m_dimensions.x, 0.0));
		m_mesh.addTexCoord(ofVec2f(0.0, m_dimensions.y));

		m_mesh.addVertex(origin + ofVec3f(m_dimensions.x, 0.0, 0.0));
		m_mesh.addVertex(origin + ofVec3f(0.0, m_dimensions.y, 0.0));
		m_mesh.addVertex(origin + ofVec3f(m_dimensions.x, m_dimensions.y, 0.0));

		m_mesh.addTexCoord(ofVec2f(m_dimensions.x, 0.0));
		m_mesh.addTexCoord(ofVec2f(0.0, m_dimensions.y));
		m_mesh.addTexCoord(ofVec2f(m_dimensions.x, m_dimensions.y));

		// Load the shaders.
		m_dropShader.load("shaders/passthru.vert", "shaders/drop3D.frag", "shaders/layer.geom");
		m_rippleShader.load("shaders/passthru.vert", "shaders/ripple3D.frag", "shaders/layer.geom");
		m_copyShader.load("shaders/passthru.vert", "shaders/copy3D.frag", "shaders/layer.geom");

		m_camera.setDistance(1000);
	}

	//--------------------------------------------------------------
	void CmbSceneGL3D::addDrop()
	{
		m_fbos[m_prevIdx].begin();
		{
			ofDefaultVec3 burstPos = ofDefaultVec3(ofRandom(m_dimensions.x), ofRandom(m_dimensions.y), ofRandom(m_dimensions.z));
			float burstThickness = 1.0f;

			m_dropShader.begin();
			{
				m_dropShader.setUniform3f("uBurst.pos", burstPos);
				m_dropShader.setUniform1f("uBurst.radius", m_radius);
				m_dropShader.setUniform1f("uBurst.thickness", burstThickness);
				//m_dropShader.printActiveUniforms();

				ofSetColor(255, 0, 0);

				int minLayer = MAX(0, burstPos.z - m_radius - burstThickness);
				int maxLayer = MIN(m_dimensions.z - 1, burstPos.z + m_radius + burstThickness);
				for (int i = minLayer; i <= maxLayer; ++i) {
					//for (int i = 0; i < m_dimensions.z; ++i) {
					m_dropShader.setUniform1i("uLayer", i);
					m_mesh.draw();
				}
			}
			m_dropShader.end();
		}
		m_fbos[m_prevIdx].end();
	}

	//--------------------------------------------------------------
	void CmbSceneGL3D::stepRipple()
	{
		m_fbos[m_tempIdx].begin();
		{
			m_rippleShader.begin();
			{
				m_rippleShader.setUniform1f("uDamping", m_damping / 10.0f + 0.9f);  // 0.9 - 1.0 range
				m_rippleShader.setUniformTexture("uPrevBuffer", m_textures[m_prevIdx].texData.textureTarget, m_textures[m_prevIdx].texData.textureID, 1);
				m_rippleShader.setUniformTexture("uCurrBuffer", m_textures[m_currIdx].texData.textureTarget, m_textures[m_currIdx].texData.textureID, 2);
				m_rippleShader.setUniform3f("uDims", m_dimensions);

				for (int i = 0; i < m_dimensions.z; ++i)
				{
					m_rippleShader.setUniform1i("uLayer", i);
					m_mesh.draw();
				}
			}
			m_rippleShader.end();
		}
		m_fbos[m_tempIdx].end();
	}

	//--------------------------------------------------------------
	void CmbSceneGL3D::copyResult()
	{
		m_fbos[m_currIdx].begin();
		{
			m_copyShader.begin();
			{
				m_copyShader.setUniformTexture("uCopyBuffer", m_textures[m_tempIdx].texData.textureTarget, m_textures[m_tempIdx].texData.textureID, 1);
				m_copyShader.setUniform3f("uDims", m_dimensions);
				{
					for (int i = 0; i < m_dimensions.z; ++i)
					{
						m_copyShader.setUniform1i("uLayer", i);
						m_mesh.draw();
					}
				}
			}
			m_copyShader.end();
		}
		m_fbos[m_currIdx].end();

		m_volumetrics.setup(&m_textures[m_currIdx], ofDefaultVec3(1, 1, 1));
	}

	//--------------------------------------------------------------
	void CmbSceneGL3D::draw()
	{
		m_volumetrics.setRenderSettings(1.0, 1.0, 1.0, 0.1);

		m_camera.begin();
		{
			m_volumetrics.drawVolume(0, 0, 0, ofGetHeight(), 0);
		}
		m_camera.end();
	}
}
