//
//  CellRenderer.cpp
//  Entropy
//
//  Created by Elias Zananiri on 2016-02-17.
//
//

#include "ofxHDF5.h"
#include "ofxImGui.h"

#include "Common.h"
#include "CellRenderer.h"

namespace ent
{
    //--------------------------------------------------------------
    CellRenderer::CellRenderer()
		: m_bRender(true)
		, m_densityMin(0.0f)
		, m_densityMax(0.0015f)
    {

    }

    //--------------------------------------------------------------
    CellRenderer::~CellRenderer()
    {
		clear();
    }

    //--------------------------------------------------------------
    void CellRenderer::setup(const std::string& folder)
    {
		clear();

		// Load all the snapshots.
		// TODO: Read snapshot folders from incoming folder.
		for (int i = 0; i < 1; ++i)
		{
			SnapshotRamses *snapshot = new SnapshotRamses();
			snapshot->setup("RAMSES_HDF5_small/");

			// Adjust the ranges.
			m_coordRange.include(snapshot->getCoordRange());
			m_sizeRange.include(snapshot->getSizeRange());
			m_densityRange.include(snapshot->getDensityRange());

			m_snapshots.push_back(snapshot);
		}

		m_currIndex = 0;

        // Set normalization values to remap to [-0.5, 0.5]
		ofDefaultVec3 coordSpan = m_coordRange.getSpan();
        m_originShift = -0.5 * coordSpan - m_coordRange.getMin();

        m_normalizeFactor = MAX(MAX(coordSpan.x, coordSpan.y), coordSpan.z);

        // Load the shaders.
        m_renderShader.setupShaderFromFile(GL_VERTEX_SHADER, "shaders/render.vert");
		m_renderShader.setupShaderFromFile(GL_FRAGMENT_SHADER, "shaders/render.frag");
		m_renderShader.bindAttribute(DENSITY_ATTRIBUTE, "density");
		m_renderShader.bindDefaults();
		m_renderShader.linkProgram();
    }

	//--------------------------------------------------------------
	void CellRenderer::clear()
	{
		for (int i = 0; i < m_snapshots.size(); ++i)
		{
			delete m_snapshots[i];
		}
		m_snapshots.clear();

		m_coordRange.clear();
		m_sizeRange.clear();
		m_densityRange.clear();
	}

    //--------------------------------------------------------------
    void CellRenderer::update()
    {

    }

    //--------------------------------------------------------------
    void CellRenderer::draw(float scale)
    {
        if (m_bRender) {
            ofSetColor(ofColor::white);
            glPointSize(1.0);

            ofPushMatrix();
            ofScale(scale / m_normalizeFactor, scale / m_normalizeFactor, scale / m_normalizeFactor);
            ofTranslate(m_originShift);
            {
                m_renderShader.begin();
				m_renderShader.setUniform1f("uDensityMin", m_densityMin * m_densityRange.getSpan());
				m_renderShader.setUniform1f("uDensityMax", m_densityMax * m_densityRange.getSpan());
				{
					m_snapshots[m_currIndex]->update(m_renderShader);
					m_snapshots[m_currIndex]->draw();
				}
				m_renderShader.end();
            }
            ofPopMatrix();
        }
    }

    //--------------------------------------------------------------
    bool CellRenderer::imGui(ofDefaultVec2& windowPos, ofDefaultVec2& windowSize)
    {
        ImGui::SetNextWindowPos(windowPos, ImGuiSetCond_Appearing);
        ImGui::SetNextWindowSize(ofDefaultVec2(380, 364), ImGuiSetCond_Appearing);
        if (ImGui::Begin("Cell Renderer", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text("Frame %lu / %lu", m_snapshots.size(), m_currIndex);
			ImGui::Text("%lu Cells", m_snapshots[m_currIndex]->getNumCells());

            if (ImGui::CollapsingHeader("Data", nullptr, true, true)) {
				ImGui::DragFloatRange2("Density Range", &m_densityMin, &m_densityMax, 0.0001f, 0.0f, 1.0f, "Min: %.4f%%", "Max: %.4f%%");
            }

            if (ImGui::CollapsingHeader("3D", nullptr, true, true)) {
                ImGui::Checkbox("Render", &m_bRender);
            }
            windowSize = ImGui::GetWindowSize();
            ImGui::End();
        }
        else {
            windowSize = ofDefaultVec2(0);
        }

        ofRectangle windowBounds(windowPos, windowSize.x, windowSize.y);
        return windowBounds.inside(ofGetMouseX(), ofGetMouseY());
    }
}
