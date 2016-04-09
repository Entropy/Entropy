#include "InflationApp.h"

namespace entropy
{
    //--------------------------------------------------------------
    void InflationApp::setup()
    {
        // Marching Cubes
        paramsMarchingCubes.setName("MARCHING CUBES");
        paramsMarchingCubes.add(resolution.set("RESOLUTION", 32, 1, 128));
        paramsMarchingCubes.add(scale.set("SCALE", 16, 1, 100));
        paramsMarchingCubes.add(threshold.set("THRESHOLD", marchingCubes.threshold, 0.0, 1.0));
        paramsMarchingCubes.add(radialClip.set("RADIAL CLIP", false));
        paramsMarchingCubes.add(fillEdges.set("FILL EDGES", false));
        paramsMarchingCubes.add(flipNormals.set("FLIP NORMALS", false));
        paramsMarchingCubes.add(smooth.set("SMOOTH", marchingCubes.getSmoothing()));
        ofAddListener(paramsMarchingCubes.parameterChangedE(), this, &InflationApp::paramsMarchingCubesChanged);

        panelMarchingCubes.setup(paramsMarchingCubes, "marching-cubes.xml");
        panelMarchingCubes.loadFromFile("marching-cubes.xml");
        panelMarchingCubes.setPosition(0, 0);

        marchingCubes.setup();
        marchingCubes.setResolution(resolution, resolution, resolution);
        marchingCubes.scale.set(resolution * scale, resolution * scale, resolution * scale);

        // Noise Field
        panelNoiseField.setup(noiseField.paramGroup, "noise-field.xml");
        panelNoiseField.loadFromFile("noise-field.xml");
        panelNoiseField.setPosition(0, panelMarchingCubes.getShape().getMaxY() + 1);

        // Render
        light.setup();
        light.setPosition(200, -200, 400);
        light.setDiffuseColor(ofColor::blueSteel);
        light.setSpecularColor(ofColor(255.f, 255.f, 255.f));

        material.setShininess(64);

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
    void InflationApp::update()
    {
        noiseField.update();

        ofVec3f centroid(marchingCubes.resX * 0.5, marchingCubes.resY * 0.5, marchingCubes.resZ * 0.5);
        float clipDistance = pow(resolution * 0.5, 2);

        for (int i = 0; i < marchingCubes.resX; ++i) {
            for (int j = 0; j < marchingCubes.resY; ++j) {
                for (int k = 0; k < marchingCubes.resZ; ++k) {
//                    if (radialClip && centroid.squareDistance(ofVec3f(i, j, k)) > clipDistance) {
//                        marchingCubes.setIsoValue(i, j, k, 0);
//                    }
                    if (fillEdges && (i == 0 || j == 0 || k == 0 ||
                                      i == marchingCubes.resX - 1 ||
                                      j == marchingCubes.resY - 1 ||
                                      k == marchingCubes.resZ - 1)) {
                        marchingCubes.setIsoValue(i, j, k, 0.99);
                    }
                    else {
                        marchingCubes.setIsoValue(i, j, k, noiseField.getValue(i, j, k));
                    }
                }
            }
        }

        marchingCubes.update();
    }

    //--------------------------------------------------------------
    void InflationApp::draw()
    {
        ofSetWindowTitle(ofToString(ofGetFrameRate(), 2) + " FPS");

        ofEnableLighting();
        ofEnableDepthTest();
        camera.begin();

        light.enable();

        if (debug) {
            ofSetColor(255, 64);

            // TODO: Optimize this with a VBO or something.

            ofPushMatrix();
            ofTranslate(-0.5 * marchingCubes.resX * scale,
                        -0.5 * marchingCubes.resY * scale,
                        -0.5 * marchingCubes.resZ * scale);

            for (int i = 0; i < marchingCubes.resX; ++i) {
                for (int j = 0; j < marchingCubes.resY; ++j) {
                    for (int k = 0; k < marchingCubes.resZ; ++k) {
                        float dim = marchingCubes.getIsoValue(i, j, k);
                        ofDrawBox(i * scale, j * scale, k * scale,
                                  dim * scale, dim * scale, dim * scale);
                    }
                }
            }
            ofPopMatrix();
        }
        else {
            ofSetColor(255);

            if (shadeNormals) {
                normalShader.begin();
                ofMatrix4x4 normalMatrix = ofMatrix4x4::getTransposedOf((ofGetCurrentMatrix(OF_MATRIX_MODELVIEW)).getInverse());
                normalShader.setUniformMatrix4f("uNormalMatrix", normalMatrix);
            }
            else {
                material.begin();
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
            else {
                material.end();
            }
        }
        
        if (drawGrid) {
            marchingCubes.drawGrid();
        }

        light.disable();

        camera.end();
        ofDisableDepthTest();
        ofDisableLighting();

        if (guiVisible) {
            panelMarchingCubes.draw();
            panelNoiseField.draw();
            panelRender.draw();
        }
    }

    //--------------------------------------------------------------
    void InflationApp::keyPressed(int key){

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
    void InflationApp::keyReleased(int key){

    }

    //--------------------------------------------------------------
    void InflationApp::mouseMoved(int x, int y ){

    }

    //--------------------------------------------------------------
    void InflationApp::mouseDragged(int x, int y, int button){

    }

    //--------------------------------------------------------------
    void InflationApp::mousePressed(int x, int y, int button){

    }

    //--------------------------------------------------------------
    void InflationApp::mouseReleased(int x, int y, int button){

    }

    //--------------------------------------------------------------
    void InflationApp::mouseEntered(int x, int y){

    }

    //--------------------------------------------------------------
    void InflationApp::mouseExited(int x, int y){

    }

    //--------------------------------------------------------------
    void InflationApp::windowResized(int w, int h){

    }

    //--------------------------------------------------------------
    void InflationApp::gotMessage(ofMessage msg){

    }

    //--------------------------------------------------------------
    void InflationApp::dragEvent(ofDragInfo dragInfo){

    }

    //--------------------------------------------------------------
    void InflationApp::paramsMarchingCubesChanged(ofAbstractParameter& param)
    {
        string paramName = param.getName();

        if (paramName == resolution.getName()) {
            marchingCubes.setResolution(resolution, resolution, resolution);
            marchingCubes.scale.set(resolution * scale, resolution * scale, resolution * scale);
        }
        else if (paramName == scale.getName()) {
            marchingCubes.scale.set(resolution * scale, resolution * scale, resolution * scale);
        }
        else if (paramName == threshold.getName()) {
            marchingCubes.threshold = threshold;
        }
        else if (paramName == smooth.getName()) {
            marchingCubes.setSmoothing(smooth);
        }
        else if (paramName == flipNormals.getName()) {
            marchingCubes.flipNormals();
        }
    }
}