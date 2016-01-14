#include "ofApp.h"

namespace entropy
{
    string surfaceTypes[] = { "noise", "spheres", "sine^2"};
    float elapsedTime;
    bool bPause = false;

    //--------------------------------------------------------------
    void ofApp::setup()
    {
        guiVisible = true;

        differentSurfaces = 0;

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
    }

    //--------------------------------------------------------------
    void ofApp::update()
    {
        noiseField.update();

        if(!bPause) elapsedTime = ofGetElapsedTimef();

        for (int i = 0; i < marchingCubes.resX; ++i) {
            for (int j = 0; j < marchingCubes.resY; ++j) {
                for (int k = 0; k < marchingCubes.resZ; ++k) {
                    marchingCubes.setIsoValue(i, j, k, noiseField.getValue(i, j, k));
                }
            }
        }

    /*
    if(differentSurfaces == 0){
        //NOISE
        float noiseStep = elapsedTime * .5;
        float noiseScale = .06;
        float noiseScale2 = noiseScale * 2.;
        for(int i=0; i<marchingCubes.resX; i++){
            for(int j=0; j<marchingCubes.resY; j++){
                for(int k=0; k<marchingCubes.resZ; k++){
                    //noise
                    float nVal = ofNoise(float(i)*noiseScale, float(j)*noiseScale, float(k)*noiseScale + noiseStep);
                    if(nVal > 0.)	nVal *= ofNoise(float(i)*noiseScale2, float(j)*noiseScale2, float(k)*noiseScale2 + noiseStep);
                    marchingCubes.setIsoValue( i, j, k, nVal );
                }
            }
        }
    }
    else if(differentSurfaces == 1){
        //SPHERES
        ofVec3f step = ofVec3f(3./marchingCubes.resX, 1.5/marchingCubes.resY, 3./marchingCubes.resZ) * PI;
        for(int i=0; i<marchingCubes.resX; i++){
            for(int j=0; j<marchingCubes.resY; j++){
                for(int k=0; k<marchingCubes.resZ; k++){;
                    float val = sin(float(i)*step.x) * sin(float(j+elapsedTime)*step.y) * sin(float(k+elapsedTime)*step.z);
                    val *= val;
                    marchingCubes.setIsoValue( i, j, k, val );
                }
            }
        }
    }
    else if(differentSurfaces == 2){
        //SIN
        float sinScale = .5;
        for(int i=0; i<marchingCubes.resX; i++){
            for(int j=0; j<marchingCubes.resY; j++){
                for(int k=0; k<marchingCubes.resZ; k++){
                    float val = sin(float(i)*sinScale) + cos(float(j)*sinScale) + sin(float(k)*sinScale + elapsedTime);
                    marchingCubes.setIsoValue( i, j, k, val * val );
                }
            }
        }
    }
     */
    
    //update the mesh
    marchingCubes.update();
    //	marchingCubes.update(threshold);

}

//--------------------------------------------------------------
void ofApp::draw()
{
    float elapsedTime = ofGetElapsedTimef();
    ofSetWindowTitle( ofToString( ofGetFrameRate() ) );

    ofEnableDepthTest();
    camera.begin();

    if (debug) {
        ofSetColor(255, 64);

        light.enable();

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

    string info = "fps:" + ofToString(ofGetFrameRate()) +
    + "\nnum vertices:" + ofToString(marchingCubes.vertexCount, 0)
    + "\nthreshold:" + ofToString(marchingCubes.threshold)
    + "\n' ' changes surface type, currently " + surfaceTypes[differentSurfaces]

    + "\n's' toggles smoothing"
    + "\n'w' toggles wireframe"
    + "\n'f' flips normals"
    + "\n'g' toggles draw grid"
    + "\n'p' toggles pause"
    + "\n'up/down' +- threshold";
    
    ofDrawBitmapString(info, 20, 20);

    if (guiVisible) {
        panelMarchingCubes.draw();
        panelNoiseField.draw();
        panelRender.draw();
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

    switch (key) {
        case 'w':
            wireframe = !wireframe;
            break;

        case 'f':
            marchingCubes.flipNormals();
            break;

        case 's':
            marchingCubes.setSmoothing( !marchingCubes.getSmoothing() );
            break;

        case 'g':
            drawGrid = !drawGrid;
            break;

        case ' ':
            differentSurfaces++;
            if(differentSurfaces>=3){
                differentSurfaces = 0;
            }
            break;
            
        case 'p':
            bPause = !bPause;
            break;

        case OF_KEY_TAB:
            guiVisible ^= 1;
            break;
            
        case OF_KEY_UP:
            marchingCubes.threshold += .03;
            break;
        case OF_KEY_DOWN:
            marchingCubes.threshold -= .03;
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