#include "ofApp.h"

namespace entropy
{
    //--------------------------------------------------------------
    void ofApp::setup()
    {
        // Marching Cubes
        paramsMarchingCubes.setName("MARCHING CUBES");
        paramsMarchingCubes.add(mcResX.set("RESOLUTION X", 32, 1, 128));
        paramsMarchingCubes.add(mcResY.set("RESOLUTION Y", 32, 1, 128));
        paramsMarchingCubes.add(mcResZ.set("RESOLUTION Z", 32, 1, 128));
        paramsMarchingCubes.add(mcScale.set("SCALE", 16, 1, 100));
        paramsMarchingCubes.add(mcThreshold.set("THRESHOLD", marchingCubes.threshold, 0.0, 1.0));
        paramsMarchingCubes.add(mcSmooth.set("SMOOTH", marchingCubes.getSmoothing()));
        paramsMarchingCubes.add(mcFlipNormals.set("FLIP NORMALS", false));
        ofAddListener(paramsMarchingCubes.parameterChangedE(), this, &ofApp::paramsMarchingCubesChanged);

        panelMarchingCubes.setup(paramsMarchingCubes, "marching-cubes.xml");
        panelMarchingCubes.loadFromFile("marching-cubes.xml");
        panelMarchingCubes.setPosition(0, 0);

        marchingCubes.setup();
        marchingCubes.setResolution(mcResX, mcResY, mcResZ);
        marchingCubes.scale.set(mcResX * mcScale, mcResY * mcScale, mcResZ * mcScale);

        // Noise Field
        panelNoiseField.setup(noiseField.paramGroup, "noise-field.xml");
        panelNoiseField.loadFromFile("noise-field.xml");
        panelNoiseField.setPosition(0, panelMarchingCubes.getShape().getMaxY() + 1);

        // Render
        light.setup();
        light.setPosition(200, -200, 400);

        paramsRender.setName("RENDER");
        paramsRender.add(debug.set("DEBUG", false));
        paramsRender.add(drawGrid.set("DRAW GRID", true));
        paramsRender.add(wireframe.set("WIREFRAME", true));
        paramsRender.add(shadeNormals.set("SHADE NORMALS", true));

        panelRender.setup(paramsRender, "render.xml");
        panelRender.loadFromFile("render.xml");
        panelRender.setPosition(0, panelNoiseField.getShape().getMaxY() + 1);
        
        normalShader.load("shaders/normalShader");

        // GUI
        guiVisible = true;
    }

    //--------------------------------------------------------------
    void ofApp::update()
    {
        noiseField.update();
        noiseField.apply(marchingCubes);

        marchingCubes.update();
    }

    //--------------------------------------------------------------
    void ofApp::draw()
    {
        ofSetWindowTitle(ofToString(ofGetFrameRate(), 2) + " FPS");

        ofEnableDepthTest();
        camera.begin();

        if (debug) {
            ofSetColor(255, 64);

            light.enable();

            // TODO: Optimize this with a VBO or something.

            ofPushMatrix();
            ofTranslate(-0.5 * marchingCubes.resX * mcScale,
                        -0.5 * marchingCubes.resY * mcScale,
                        -0.5 * marchingCubes.resZ * mcScale);

            for (int i = 0; i < marchingCubes.resX; ++i) {
                for (int j = 0; j < marchingCubes.resY; ++j) {
                    for (int k = 0; k < marchingCubes.resZ; ++k) {
                        float dim = marchingCubes.getIsoValue(i, j, k);
                        ofDrawBox(i * mcScale, j * mcScale, k * mcScale,
                                  dim * mcScale, dim * mcScale, dim * mcScale);
                    }
                }
            }
            ofPopMatrix();

            light.disable();
        }
        else {
            ofSetColor(255);

            if (shadeNormals) {
                normalShader.begin();
                ofMatrix4x4 normalMatrix = ofMatrix4x4::getTransposedOf((ofGetCurrentMatrix(OF_MATRIX_MODELVIEW)).getInverse());
                normalShader.setUniformMatrix4f("uNormalMatrix", normalMatrix);
            }

            if (wireframe) {
                marchingCubes.drawWireframe();
            }
            else {
                marchingCubes.draw();
            }

            if (shadeNormals) {
                normalShader.end();
            }
        }
        
        if (drawGrid) {
            marchingCubes.drawGrid();
        }
        
        camera.end();
        ofDisableDepthTest();
        
        if (guiVisible) {
            panelMarchingCubes.draw();
            panelNoiseField.draw();
            panelRender.draw();
        }
    }

    //--------------------------------------------------------------
    void ofApp::keyPressed(int key){

        switch (key) {
            case '`':
                guiVisible ^= 1;
                break;

            case OF_KEY_TAB:
                ofToggleFullscreen();
                break;

            default:
                break;
        }

    }

    //--------------------------------------------------------------
    void ofApp::keyReleased(int key){

    }

    //--------------------------------------------------------------
    void ofApp::mouseMoved(int x, int y ){

    }

    //--------------------------------------------------------------
    void ofApp::mouseDragged(int x, int y, int button){

    }

    //--------------------------------------------------------------
    void ofApp::mousePressed(int x, int y, int button){

    }

    //--------------------------------------------------------------
    void ofApp::mouseReleased(int x, int y, int button){

    }

    //--------------------------------------------------------------
    void ofApp::mouseEntered(int x, int y){

    }

    //--------------------------------------------------------------
    void ofApp::mouseExited(int x, int y){

    }

    //--------------------------------------------------------------
    void ofApp::windowResized(int w, int h){

    }

    //--------------------------------------------------------------
    void ofApp::gotMessage(ofMessage msg){

    }

    //--------------------------------------------------------------
    void ofApp::dragEvent(ofDragInfo dragInfo){

    }

    //--------------------------------------------------------------
    void ofApp::paramsMarchingCubesChanged(ofAbstractParameter& param)
    {
        string paramName = param.getName();

        if (paramName == mcResX.getName() ||
            paramName == mcResY.getName() ||
            paramName == mcResZ.getName()) {
            marchingCubes.setResolution(mcResX, mcResY, mcResZ);
            marchingCubes.scale.set(mcResX * mcScale, mcResY * mcScale, mcResZ * mcScale);
        }
        else if (paramName == mcScale.getName()) {
            marchingCubes.scale.set(mcResX * mcScale, mcResY * mcScale, mcResZ * mcScale);
        }
        else if (paramName == mcThreshold.getName()) {
            marchingCubes.threshold = mcThreshold;
        }
        else if (paramName == mcSmooth.getName()) {
            marchingCubes.setSmoothing(mcSmooth);
        }
        else if (paramName == mcFlipNormals.getName()) {
            marchingCubes.flipNormals();
        }
    }
}