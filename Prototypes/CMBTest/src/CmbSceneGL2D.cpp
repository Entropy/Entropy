#include "CmbSceneGL2D.h"

namespace ent
{
	//--------------------------------------------------------------
	void CmbSceneGL2D::setup()
	{		
		CmbScene::setup();
		
		// Allocate the textures and buffers.
		for (int i = 0; i < 3; ++i) {
			m_textures[i].allocate(m_dimensions.x, m_dimensions.y, GL_RGBA32F);

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
		m_mesh.clear();
		m_mesh.setMode(OF_PRIMITIVE_TRIANGLE_FAN);

		m_mesh.addVertex(ofDefaultVec3(0, 0));
		m_mesh.addVertex(ofDefaultVec3(m_dimensions.x, 0));
		m_mesh.addVertex(ofDefaultVec3(m_dimensions.x, m_dimensions.y));
		m_mesh.addVertex(ofDefaultVec3(0, m_dimensions.y));

		m_mesh.addTexCoord(ofDefaultVec2(0, 0));
		m_mesh.addTexCoord(ofDefaultVec2(m_dimensions.x, 0));
		m_mesh.addTexCoord(ofDefaultVec2(m_dimensions.x, m_dimensions.y));
		m_mesh.addTexCoord(ofDefaultVec2(0, m_dimensions.y));

		// Load the shader.
		m_shader.load("shaders/passthru.vert", "shaders/ripple.frag");
	}

	//--------------------------------------------------------------
	void CmbSceneGL2D::addDrop()
	{
		m_fbos[m_prevIdx].begin();
		{
			ofPushMatrix();
			{
				ofScale(1.0, -1.0, 1.0);
				ofTranslate(0.0, m_dimensions.y * -1.0, 0.0);
			
				ofPushStyle();
				{
					ofSetColor(m_dropColor);
					ofNoFill();

					ofDrawCircle(ofRandomWidth(), ofRandomHeight(), m_radius);
				}
				ofPopStyle();
			}
			ofPopMatrix();
		}
		m_fbos[m_prevIdx].end();
	}

	//--------------------------------------------------------------
	void CmbSceneGL2D::stepRipple()
	{
		m_fbos[m_tempIdx].begin();
		{
			m_shader.begin();
			m_shader.setUniform1f("uDamping", m_damping / 10.0f + 0.9f);  // 0.9 - 1.0 range
			m_shader.setUniformTexture("uPrevBuffer", m_textures[m_prevIdx], 1);
			m_shader.setUniformTexture("uCurrBuffer", m_textures[m_currIdx], 2);
			{
				m_mesh.draw();
			}
			m_shader.end();
		}
		m_fbos[m_tempIdx].end();
	}

	//--------------------------------------------------------------
	void CmbSceneGL2D::copyResult()
	{
		m_fbos[m_currIdx].begin();
		{
			m_textures[m_tempIdx].bind();
			{
				m_mesh.draw();
			}
			m_textures[m_tempIdx].unbind();
		}
		m_fbos[m_currIdx].end();
	}

	//--------------------------------------------------------------
	void CmbSceneGL2D::draw()
	{
		m_textures[m_prevIdx].draw(0, 0);
	}
}
