#include "ofApp.h"

#include "ofxGaussianMapTexture.h"

typedef struct
{
    float longitude;
    float latitude;
    float radius;
} Coordinate;

//--------------------------------------------------------------
void ofApp::setup()
{
    ofSetLogLevel(OF_LOG_NOTICE);
    ofSetDataPathRoot("../Resources/data/");
    ofDisableArbTex();
    ofBackground(ofColor::black);

    pointSize = 8.0f;
    bUseSprites = true;
    scale = 1.0f;

    // Load initial data.
    string filename = "sample_contig.hdf5";
    loadData(filename);

    // Build the texture.
    ofxCreateGaussianMapTexture(texture, 32);

    // Load the shader.
    shader.setupShaderFromFile(GL_VERTEX_SHADER, "shaders/render.vert");
    shader.setupShaderFromFile(GL_FRAGMENT_SHADER, "shaders/render.frag");
    shader.bindAttribute(MASS_ATTRIBUTE, "mass");
    shader.bindDefaults();
    shader.linkProgram();

    bGuiVisible = true;
}

//--------------------------------------------------------------
void ofApp::loadData(const string& filePath)
{
    int stride = 1;

    ofxHDF5File h5File;
    h5File.open(filePath, true);
    ofxHDF5GroupPtr h5Group = h5File.loadGroup("PartType6");

    ofxHDF5DataSetPtr coordsDataSet = h5Group->loadDataSet("Coordinates");
    int coordsCount = coordsDataSet->getDimensionSize(0) / stride;
    coordsDataSet->setHyperslab(0, coordsCount, stride);
    //
    vector<Coordinate> coordsData(coordsCount);
    coordsDataSet->read(coordsData.data());

    // Load the mass data.
    ofxHDF5DataSetPtr massesDataSet = h5Group->loadDataSet("Masses");
    int massesCount = massesDataSet->getDimensionSize(0) / stride;
    massesDataSet->setHyperslab(0, massesCount, stride);

    vector<float> massesData(massesCount);
    massesDataSet->read(massesData.data());

    // Convert the position data to Cartesian coordinates.
    vector<ofVec3f> vertices(coordsCount);
    for (int i = 0; i < vertices.size(); ++i) {
        vertices[i].x = coordsData[i].radius * cos(ofDegToRad(coordsData[i].latitude)) * cos(ofDegToRad(coordsData[i].longitude));
        vertices[i].y = coordsData[i].radius * cos(ofDegToRad(coordsData[i].latitude)) * sin(ofDegToRad(coordsData[i].longitude));
        vertices[i].z = coordsData[i].radius * sin(ofDegToRad(coordsData[i].latitude));
    }

    // Upload everything to the VBO.
    vboMesh.clear();
    vboMesh.addVertices(vertices);
    vboMesh.getVbo().setAttributeData(MASS_ATTRIBUTE, massesData.data(), 1, massesData.size(), GL_STATIC_DRAW, 0);
}

//--------------------------------------------------------------
void ofApp::imGui()
{
    gui.begin();
    {
        ImGui::SetNextWindowPos(ofVec2f(10, 10));
        ImGui::SetNextWindowSize(ofVec2f(360, 140), ImGuiSetCond_Once);
        if (ImGui::Begin("FITS", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("%.1f FPS (%.3f ms/frame)", ofGetFrameRate(), 1000.0f / ImGui::GetIO().Framerate);

            if (ImGui::CollapsingHeader("Data", nullptr, true, true)) {
                ImGui::Text("%lu Vertices", vboMesh.getNumVertices());
                if (ImGui::Button("Load File")) {
                    ofFileDialogResult dialogResult = ofSystemLoadDialog("Select a FITS file:", false);
                    if (dialogResult.bSuccess) {
                        loadData(dialogResult.getPath());
                    }
                }
            }

            if (ImGui::CollapsingHeader("Render", nullptr, true, true)) {
                ImGui::SliderFloat("Scale", &scale, 1.0f, 20.0f);
                ImGui::SliderFloat("Point Size", &pointSize, 0.1f, 64.0f);
                ImGui::Checkbox("Use Sprites", &bUseSprites);
            }

            ImGui::End();
        }
    }
    gui.end();
}

//--------------------------------------------------------------
void ofApp::update()
{

}

//--------------------------------------------------------------
void ofApp::draw()
{
    ofSetColor(ofColor::white);

    glEnable(GL_POINT_SMOOTH);
    glPointSize(pointSize);

    cam.setNearClip(0);
    cam.setFarClip(FLT_MAX);

    cam.begin();
    {
        ofPushMatrix();
        ofScale(scale, scale, scale);
        {
            ofEnableBlendMode(OF_BLENDMODE_SCREEN);

            if (bUseSprites) {
                shader.begin();
                shader.setUniformTexture("texx", texture, 1);
                shader.setUniform1f("pointSize", pointSize);
                ofEnablePointSprites();
            }
            else {
                glPointSize(pointSize);
            }

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

        ofDrawAxis(20);
    }
    cam.end();

    if (bGuiVisible) {
        imGui();
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
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
