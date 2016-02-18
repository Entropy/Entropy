//
//  VolumeRenderer.cpp
//  Entropy
//
//  Created by Elias Zananiri on 2016-02-17.
//
//

#include "ofxImGui.h"

#include "Common.h"
#include "VolumeRenderer.h"

namespace ent
{
    //--------------------------------------------------------------
    VolumeRenderer::VolumeRenderer()
    : bDoRender(true)
    {

    }

    //--------------------------------------------------------------
    VolumeRenderer::~VolumeRenderer()
    {

    }

    //--------------------------------------------------------------
    void VolumeRenderer::loadVolume(const string& folder)
    {
        ofLogNotice("VolumeRenderer::loadVolume") << "Loading volume folder " << folder;

        imageSequencePlayer.init(folder + "/" + kSliceFilePrefix, kSliceFileNumPadding, kSliceFileExt, 0);
        int volWidth = imageSequencePlayer.getWidth();
        int volHeight = imageSequencePlayer.getHeight();
        int volDepth = imageSequencePlayer.getSequenceLength();
        ofLogNotice("VolumeRenderer::loadVolume") << "Volume buffer with dimensions " << volWidth << "x" << volHeight << "x" << volDepth;

        int channels = imageSequencePlayer.getPixels().getNumChannels();

        unsigned char * volumeData = new unsigned char[volWidth * volHeight * volDepth * 4];
        for (int z = 0; z < volDepth; ++z) {
            imageSequencePlayer.loadFrame(z);
            unsigned char * frameData = imageSequencePlayer.getPixels().getData();
            if (channels == 4) {
                int vdx = (z * volWidth * volHeight) * 4;
                memcpy(&volumeData[vdx], frameData, volWidth * volHeight * 4);
            }
            else {
                for (int y = 0; y < volHeight; ++y) {
                    for (int x = 0; x < volWidth; ++x) {
                        int vdx = (x + volWidth * y + z * volWidth * volHeight) * 4;
                        int fdx = (x + volWidth * y) * channels;
                        if (channels == 1) {
                            // Color is white, luminance is alpha.
                            volumeData[vdx + 0] = 255.0;
                            volumeData[vdx + 1] = 255.0;
                            volumeData[vdx + 2] = 255.0;
                            volumeData[vdx + 3] = frameData[fdx];
                        }
                        else if (channels == 2) {
                            // Color is luminance, alpha is alpha.
                            volumeData[vdx + 0] = frameData[fdx + 0];
                            volumeData[vdx + 1] = frameData[fdx + 0];
                            volumeData[vdx + 2] = frameData[fdx + 0];
                            volumeData[vdx + 3] = frameData[fdx + 1];
                        }
                        else if (channels == 3) {
                            // Color is white, red is alpha.
                            volumeData[vdx + 0] = 255.0;
                            volumeData[vdx + 1] = 255.0;
                            volumeData[vdx + 2] = 255.0;
                            volumeData[vdx + 3] = frameData[fdx];
                        }
                    }
                }
            }
        }

        volumetrics.setup(volWidth, volHeight, volDepth, ofVec3f(1.0, 1.0, 1.0));
        volumetrics.updateVolumeData(volumeData, volWidth, volHeight, volDepth, 0, 0, 0);

        xyQuality = volumetrics.getXyQuality();
        zQuality = volumetrics.getZQuality();
        threshold = volumetrics.getThreshold();
        density = volumetrics.getDensity();
        filterMode = GL_LINEAR;
    }

    //--------------------------------------------------------------
    void VolumeRenderer::draw(float scale)
    {
        if (bDoRender && volumetrics.isInitialized()) {
            volumetrics.drawVolume(0, 0, 0, scale, 0);

//                ofPushMatrix();
//                ofScale(scale, scale, scale);
//                ofTranslate(-0.5, -0.5, -0.5);
//                //    ofEnableDepthTest();
//                ofNoFill();
//                volumetrics.volVbo.drawElements(GL_TRIANGLES, 36);
//                //    ofDisableDepthTest();
//                ofFill();
//                ofPopMatrix();
        }
    }

    //--------------------------------------------------------------
    bool VolumeRenderer::imGui(ofVec2f& windowPos, ofVec2f& windowSize)
    {
        ImGui::SetNextWindowPos(windowPos, ImGuiSetCond_Appearing);
        ImGui::SetNextWindowSize(ofVec2f(380, 380), ImGuiSetCond_Appearing);
        if (ImGui::Begin("Volume Renderer", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
//            ImGui::Text("%.1f FPS (%.3f ms/frame)", ofGetFrameRate(), 1000.0f / ImGui::GetIO().Framerate);
//            ImGui::Text("%lu Indices / %lu Vertices", vboMesh.getNumIndices(), vboMesh.getNumVertices());
//
            if (ImGui::CollapsingHeader("Data", nullptr, true, true)) {
                if (ImGui::Button("Import")) {
                    ofFileDialogResult dialogResult = ofSystemLoadDialog("Select a folder containing slices:", true, ofToDataPath(""));
                    if (dialogResult.bSuccess) {
                        loadVolume(dialogResult.getPath());
                    }
                }
            }
//
            if (ImGui::CollapsingHeader("Render", nullptr, true, true)) {
                ImGui::Checkbox("Enabled", &bDoRender);
                if (ImGui::SliderFloat("XY Quality", &xyQuality, 0.0f, 1.0f)) {
                    volumetrics.setXyQuality(xyQuality);
                }
                if (ImGui::SliderFloat("Z Quality", &zQuality, 0.0f, 1.0f)) {
                    volumetrics.setZQuality(zQuality);
                }
                if (ImGui::SliderFloat("Threshold", &threshold, 0.0f, 1.0f)) {
                    volumetrics.setThreshold(threshold);
                }
                if (ImGui::SliderFloat("Density", &density, 0.0f, 1.0f)) {
                    volumetrics.setDensity(density);
                }
                ImGui::Text("Filter Mode");
                if (ImGui::RadioButton("Linear", &filterMode, GL_LINEAR)) {
                    volumetrics.setVolumeTextureFilterMode(filterMode);
                }
                ImGui::SameLine();
                if (ImGui::RadioButton("Nearest", &filterMode, GL_NEAREST)) {
                    volumetrics.setVolumeTextureFilterMode(filterMode);
                }
            }
            
            windowSize.set(ImGui::GetWindowSize());
            ImGui::End();
        }
        else {
            windowSize.set(0);
        }

        ofRectangle windowBounds(windowPos, windowSize.x, windowSize.y);
        return windowBounds.inside(ofGetMouseX(), ofGetMouseY());
    }
}
