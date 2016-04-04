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
    : stride(1)
    , bNeedsIndices(true)
    , bRender3D(true)
    , pointSize(1.5f)
    , densityMin(0.0f)
    , densityMax(0.0015f)
    , bNeedsBins(true)
    , bCycleBins(false)
    , bExportFiles(false)
    , binPower(8)
    , binIndex(0)
    , renderIndex(-1)
    , pointAdjust(2.55f)
    , bBinDebug2D(false)
    , bBinDebug3D(true)
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
        originShift = -0.5 * coordSpan - m_coordRange.getMin();

        normalizeFactor = MAX(MAX(coordSpan.x, coordSpan.y), coordSpan.z);

        // Load the shaders.
        renderShader.setupShaderFromFile(GL_VERTEX_SHADER, "shaders/render.vert");
        renderShader.setupShaderFromFile(GL_FRAGMENT_SHADER, "shaders/render.frag");
        renderShader.bindAttribute(DENSITY_ATTRIBUTE, "density");
        renderShader.bindDefaults();
        renderShader.linkProgram();

        //renderShader.begin();
        //{
        //    renderShader.setUniformTexture("uTransform", bufferTexture, 0);
        //}
        //renderShader.end();

        sliceShader.setupShaderFromFile(GL_VERTEX_SHADER, "shaders/slice.vert");
        sliceShader.setupShaderFromFile(GL_FRAGMENT_SHADER, "shaders/slice.frag");
        sliceShader.bindAttribute(DENSITY_ATTRIBUTE, "density");
        sliceShader.bindDefaults();
        sliceShader.linkProgram();
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
        if (bNeedsIndices) {
            rebuildIndices();
        }
        if (bNeedsBins) {
            rebuildBins();
        }

//        ofEnablePointSprites();

//        if ((bBinDebug2D || bExportFiles) && binIndex != renderIndex) {
//            float minDepth = coordRange.getMin().z + binIndex * binSliceZ;
//            float maxDepth = minDepth + binSliceZ;
//
//            ofLogNotice() << "Rendering bin " << binIndex << " to texture.";
//            binFbo.begin();
//            {
//                ofClear(0, 0);
//                ofSetColor(ofColor::white);
//                glPointSize(1.0);
//
//                sliceShader.begin();
//                sliceShader.setUniform1f("pointAdjust", pointAdjust);
//                sliceShader.setUniform3f("minCoord", coordRange.getMin());
//                sliceShader.setUniform3f("maxCoord", coordRange.getMax());
//                sliceShader.setUniform1f("binSizeX", binSizeX);
//                sliceShader.setUniform1f("binSizeY", binSizeY);
//                sliceShader.setUniform1f("minDepth", minDepth);
//                sliceShader.setUniform1f("maxDepth", maxDepth);
////                sliceShader.setUniform1f("minDensity", densityRange.getMin());
////                sliceShader.setUniform1f("maxDensity", densityRange.getMax());
//                sliceShader.setUniform1f("minDensity", densityMin * densityRange.getSpan());
//                sliceShader.setUniform1f("maxDensity", densityMax * densityRange.getSpan());
//                {
//                    vboMesh.getVbo().drawElements(GL_POINTS, vboMesh.getNumIndices());
//                }
//                sliceShader.end();
//            }
//            binFbo.end();
//
//            renderIndex = binIndex;
//
//            if (bExportFiles) {
//                ofLogVerbose() << "Saving texture " << binIndex << " to disk.";
//                binFbo.readToPixels(binPixels);
//                ofSaveImage(binPixels, exportFolder + "/" + kSliceFilePrefix + ofToString(binIndex, kSliceFileNumPadding, '0') + kSliceFileExt);
//            }
//        }
    }

    //--------------------------------------------------------------
    void CellRenderer::rebuildIndices()
    {
//        vboMesh.clearIndices();
//        for (int i = 0; i < vboMesh.getNumVertices(); i += stride) {
//            vboMesh.addIndex(i);
//        }
//        bNeedsIndices = false;
    }

    //--------------------------------------------------------------
    void CellRenderer::rebuildBins()
    {
//        // Build bins by depth.
//        binSizeX = binSizeY = binSizeZ = pow(2, binPower);
//        binSliceZ = (coordRange.getSpan().z) / binSizeZ;
//
//        ofLogNotice() << "Bin size is [" << binSizeX << "x" << binSizeY << "x" << binSizeZ << "] with slice " << binSliceZ;
//
//        // Allocate FBO.
//        binFbo.allocate(binSizeX, binSizeY);
////        binIndex = 0;
//        renderIndex = -1;
//
//        bNeedsBins = false;
    }

    //--------------------------------------------------------------
    void CellRenderer::draw(float scale)
    {
        if (bRender3D) {
            ofSetColor(ofColor::white);
            glPointSize(1.0);

            ofPushMatrix();
            ofScale(scale / normalizeFactor, scale / normalizeFactor, scale / normalizeFactor);
            ofTranslate(originShift);
            {
                renderShader.begin();
                renderShader.setUniform1f("uDensityMin", densityMin * m_densityRange.getSpan());
                renderShader.setUniform1f("uDensityMax", densityMax * m_densityRange.getSpan());
                //if (bBinDebug3D) {
                //    renderShader.setUniform1f("uDebugMin", coordRange.getMin().z + binIndex * binSliceZ);
                //    renderShader.setUniform1f("uDebugMax", coordRange.getMin().z + (binIndex + 1) * binSliceZ);
                //}
                //else {
                renderShader.setUniform1f("uDebugMin", FLT_MAX);
                renderShader.setUniform1f("uDebugMax", FLT_MIN);
                //}
				{
					m_snapshots[m_currIndex]->update(renderShader);
					m_snapshots[m_currIndex]->draw();
				}
                renderShader.end();
            }
            ofPopMatrix();
        }

        if (bCycleBins) {
            binIndex = (binIndex + 1) % binSizeZ;
            
            if (bExportFiles && binIndex == 0) {
                bExportFiles = false;
                bCycleBins = false;
            }
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
                if (ImGui::SliderInt("Stride", &stride, 1, 128)) {
                    bNeedsIndices = true;
                }
                if (ImGui::DragFloatRange2("Density Range", &densityMin, &densityMax, 0.0001f, 0.0f, 1.0f, "Min: %.4f%%", "Max: %.4f%%")) {
                    renderIndex = -1;
                }
            }

            if (ImGui::CollapsingHeader("3D", nullptr, true, true)) {
                ImGui::Checkbox("Render", &bRender3D);
                ImGui::SliderFloat("Point Size", &pointSize, 0.1f, 64.0f);
            }

            if (ImGui::CollapsingHeader("Export", nullptr, true, true)) {
                if (ImGui::SliderInt("Bin Power", &binPower, 0, 10)) {
                    bNeedsBins = true;
                }
                ImGui::SliderInt("Bin Index", &binIndex, 0, binSizeZ - 1);
                ImGui::Checkbox("Cycle Bins", &bCycleBins);
                if (ImGui::Checkbox("Export Files", &bExportFiles)) {
                    exportFolder = ofSystemTextBoxDialog("Type in a name for the output folder:");
                    if (exportFolder.length()) {
                        binIndex = 0;
                        renderIndex = -1;
                        bCycleBins = true;
                    }
                }
                if (ImGui::SliderFloat("Point Adjust", &pointAdjust, 0.0f, 5.0f)) {
                    renderIndex = -1;
                }
                ImGui::Checkbox("Debug 3D", &bBinDebug3D);
                if (ImGui::TreeNode("Debug 2D")) {
                    bBinDebug2D = true;
                    ImGui::Image((ImTextureID)(uintptr_t)binFbo.getTexture().getTextureData().textureID, ofDefaultVec2(binSizeX, binSizeY));
                    ImGui::TreePop();
                }
                else {
                    bBinDebug2D = false;
                }
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
