#include "ofxGaussianMapTexture.h"

#include "ExpansionApp.h"

namespace entropy
{
    //--------------------------------------------------------------
    void ExpansionApp::setup()
    {
        ofSetLogLevel(OF_LOG_VERBOSE);
        ofSetDataPathRoot("../Resources/data/");
        ofDisableArbTex();
        ofBackground(ofColor::black);

        // Default values.
        size = 10.0f;
        resolution = 2;

        type = 0;
        speed = 1.0f;

        camera = 1;
        pointSize = 4.0f;
        bUseSprites = true;

        // Load coordinates from the data file.
        ofxHDF5File file("snap_subbox3_1425.hdf5", true);
        ofxHDF5GroupPtr group = file.loadGroup("PartType1");
        ofxHDF5DataSetPtr dataSet = group->loadDataSet("Coordinates");

        int offset = 0;
        int count = dataSet->getDimensionSize(0);
        int stride = 1;

        vector<ofVec3f> vertices(count);
        dataSet->setHyperslab(offset, count, stride);
        dataSet->read(vertices.data());

        // Get the data bounds.
        ofVec3f minCoord(FLT_MAX, FLT_MAX, FLT_MAX);
        ofVec3f maxCoord(FLT_MIN, FLT_MIN, FLT_MIN);
        for (ofVec3f& v : vertices) {
            minCoord.x = min(minCoord.x, v.x);
            minCoord.y = min(minCoord.y, v.y);
            minCoord.z = min(minCoord.z, v.z);

            maxCoord.x = max(maxCoord.x, v.x);
            maxCoord.y = max(maxCoord.y, v.y);
            maxCoord.z = max(maxCoord.z, v.z);
        }

        float range = (maxCoord.x - minCoord.x);
        range = MAX(range, maxCoord.y - minCoord.y);
        range = MAX(range, maxCoord.z - minCoord.z);

        // Remap the coordinates to [-0.5 0.5]
        ofVec3f shift = (maxCoord - minCoord) * -0.5 - minCoord;
        ofLogVerbose("ExpansionApp::setup") << "Coords in range (" << minCoord << ") to (" << maxCoord << ")";
        ofLogVerbose("ExpansionApp::setup") << "Shift = " << shift;
        ofLogVerbose("ExpansionApp::setup") << "Range = " << range;

        for (ofVec3f& v : vertices) {
            v += shift;
            v /= range;
        }

        // Upload the data to the VBO.
        vboMesh.clear();
        vboMesh.addVertices(vertices);

        // Load the shader and texture for rendering.
        shader.load("shaders/billboard");
        ofxCreateGaussianMapTexture(texture, 32);

        // Setup cameras.
        easyCam.setNearClip(0.0);
        easyCam.setFarClip(FLT_MAX);
        headCam.setup();

        // Set startup flags.
        bRebuildBox = true;
        bPaused = false;
        bReset = true;
        bGuiVisible = true;
    }

    //--------------------------------------------------------------
    void ExpansionApp::imGui()
    {
        gui.begin();
        {
            ImGui::SetNextWindowPos(ofVec2f(10, 10));
            ImGui::SetNextWindowSize(ofVec2f(320, 460), ImGuiSetCond_Once);
            if (ImGui::Begin("Expansion")) {
                ImGui::Text("%.1f FPS (%.3f ms/frame)", ofGetFrameRate(), 1000.0f / ImGui::GetIO().Framerate);

                if (ImGui::CollapsingHeader("Box", nullptr, true, true)) {
                    ImGui::Checkbox("Draw Grid", &bDrawGrid);
                    if (ImGui::SliderFloat("Size", &size, 2.0f, 200.0f)) {
                        bRebuildBox = true;
                    }
                    if (ImGui::SliderInt("Resolution", &resolution, 1, 128)) {
                        bRebuildBox = true;
                    }
                }

                if (ImGui::CollapsingHeader("Scale", nullptr, true, true)) {
                    ImGui::Checkbox("Reset", &bReset);
                    ImGui::Checkbox("Paused", &bPaused);

                    ImGui::SliderFloat("Speed", &speed, 0.0f, 10.0f);

                    ImGui::Text("Type");
                    ImGui::RadioButton("Inflation / Dark Energy", &type, 0);
                    ImGui::RadioButton("Radiation Domination", &type, 1);
                    ImGui::RadioButton("Matter Domination", &type, 2);
                }

                if (ImGui::CollapsingHeader("Render", nullptr, true, true)) {
                    ImGui::SliderFloat("Point Size", &pointSize, 0.1f, 64.0f);
                    ImGui::Checkbox("Use Sprites", &bUseSprites);

                    ImGui::Text("Camera");
                    ImGui::RadioButton("Origin", &camera, 0);
                    ImGui::RadioButton("God", &camera, 1);
                }
            }
            ImGui::End();
        }
        gui.end();
    }

    //--------------------------------------------------------------
    void ExpansionApp::update()
    {
        if (bRebuildBox) {
            box.set(size, size, size, resolution, resolution, resolution);
            bRebuildBox = false;
        }

        if (bReset) {
            scale = 1.0;
            bReset = false;
        }

        // Ignore presses over the GUI.
//        bool bMousePressed = ofGetMousePressed() && (!bGuiVisible || !guiPanel.getShape().inside(ofGetMouseX(), ofGetMouseY()));

        if (!bPaused) {
            // Update the scale.
            dt = ofGetLastFrameTime();
            if (type == 0) {
                scale += exp(dt) * speed;
            }
            else if (type == 1) {
                scale += pow(dt, 0.5f) * speed;
            }
            else if (type == 2) {
                scale += pow(dt, 2.0f/3.0f) * speed;
            }
        }

        // Update camera position.
        headCam.setPosition(ofVec3f::zero());
    }

    //--------------------------------------------------------------
    void ExpansionApp::draw()
    {
        ofSetColor(ofColor::white);

        if (camera == 0) {
            headCam.begin();
        }
        else {
            easyCam.begin();
        }
        {
            ofPushMatrix();
            ofScale(scale, scale, scale);
            {
                if (bDrawGrid) {
                    // Draw the universe.
                    box.draw(OF_MESH_WIREFRAME);
                }


            ofPushMatrix();
            ofScale(size, size, size);
            {
                // Draw all particles.
                ofEnableBlendMode(OF_BLENDMODE_SCREEN);

                if (bUseSprites) {
                    shader.begin();
                    shader.setUniformTexture("tex", texture, 1);
                    shader.setUniform1f("pointSize", pointSize);
                    ofEnablePointSprites();
                }
                else {
                    glPointSize(pointSize);
                }

                ofSetColor(ofColor::white);
                vboMesh.draw(OF_MESH_POINTS);

                if (bUseSprites) {
                    ofDisablePointSprites();
                    shader.end();
                }
                else {
                    glPointSize(1.0f);
                }

                ofEnableBlendMode(OF_BLENDMODE_ALPHA);
            }
            ofPopMatrix();
            }
            ofPopMatrix();

            ofDrawAxis(size);
        }
        if (camera == 0) {
            headCam.end();
        }
        else {
            easyCam.end();
        }

        if (bGuiVisible) {
            imGui();
        }
    }

    //--------------------------------------------------------------
    void ExpansionApp::keyPressed(int key)
    {
        switch (key) {
            case '`':
                bGuiVisible ^= 1;
                break;

            case OF_KEY_TAB:
                ofToggleFullscreen();
                break;

            default:
                break;
        }
    }

    //--------------------------------------------------------------
    void ExpansionApp::keyReleased(int key){

    }

    //--------------------------------------------------------------
    void ExpansionApp::mouseMoved(int x, int y ){

    }

    //--------------------------------------------------------------
    void ExpansionApp::mouseDragged(int x, int y, int button){

    }

    //--------------------------------------------------------------
    void ExpansionApp::mousePressed(int x, int y, int button){

    }

    //--------------------------------------------------------------
    void ExpansionApp::mouseReleased(int x, int y, int button){

    }

    //--------------------------------------------------------------
    void ExpansionApp::mouseEntered(int x, int y){

    }

    //--------------------------------------------------------------
    void ExpansionApp::mouseExited(int x, int y){

    }

    //--------------------------------------------------------------
    void ExpansionApp::windowResized(int w, int h)
    {
        bReset = true;
    }

    //--------------------------------------------------------------
    void ExpansionApp::gotMessage(ofMessage msg){
        
    }
    
    //--------------------------------------------------------------
    void ExpansionApp::dragEvent(ofDragInfo dragInfo){ 
        
    }
}
