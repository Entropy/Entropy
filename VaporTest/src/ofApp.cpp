#include "ofApp.h"
#include "H5Cpp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
    ofBackground(ofColor::black);
    ofSetLogLevel(OF_LOG_NOTICE);

    pointSize = 1.0f;
    densityMin = 0.0f;
    densityMax = 0.1f;
    scale = 1024.0f;

    int stride = 50;
    vector<float> posX, posY, posZ, density;
    loadDataSet("x.h5", posX, stride, true);
    loadDataSet("y.h5", posY, stride, true);
    loadDataSet("z.h5", posZ, stride, true);
    loadDataSet("density.h5", density, stride, false);

    ofVec3f minCoord(FLT_MAX);
    ofVec3f maxCoord(FLT_MIN);
    float minDensity = FLT_MAX;
    float maxDensity = FLT_MIN;
    for (int i = 0; i < posX.size(); ++i) {
        minCoord.x = min(minCoord.x, posX[i]);
        minCoord.y = min(minCoord.y, posY[i]);
        minCoord.z = min(minCoord.z, posZ[i]);
        minDensity = min(minDensity, density[i]);

        maxCoord.x = max(maxCoord.x, posX[i]);
        maxCoord.y = max(maxCoord.y, posY[i]);
        maxCoord.z = max(maxCoord.z, posZ[i]);
        maxDensity = max(maxDensity, density[i]);
    }

    cout << "From: (" << minCoord << ") => " << minDensity << endl
         << "  To: (" << maxCoord << ") => " << maxDensity << endl;

    float range = (maxCoord.x - minCoord.x);
    range = MAX(range, maxCoord.y - minCoord.y);
    range = MAX(range, maxCoord.z - minCoord.z);

    // Remap the coordinates to [-0.5 0.5]
    ofVec3f shift = (maxCoord - minCoord) * -0.5 - minCoord;
    ofLogVerbose("ExpansionApp::setup") << "Coords in range (" << minCoord << ") to (" << maxCoord << ")";
    ofLogVerbose("ExpansionApp::setup") << "Shift = " << shift;
    ofLogVerbose("ExpansionApp::setup") << "Range = " << range;

    vboMesh.getVertices().resize(posX.size());
    vboMesh.getColors().resize(posX.size());
    for (int i = 0; i < posX.size(); ++i) {
        ofVec3f v(posX[i], posY[i], posZ[i]);
        v += shift;
        v /= range;
        vboMesh.setVertex(i, v);

        float alpha = ofMap(density[i], minDensity, maxDensity, 0.0f, 1.0f);
        vboMesh.setColor(i, ofFloatColor(1.0, 1.0, 1.0, alpha));
    }

    shader.load("shaders/render");

    bGuiVisible = true;
}

//--------------------------------------------------------------
void ofApp::loadDataSet(const string& filename, vector<float>& data, int stride, bool bExponential)
{
    ofxHDF5File h5File;
    h5File.open(filename, true);
    cout << "File '" << filename << "' has " << h5File.getNumDataSets() << " datasets" << endl;

    for (int i = 0; i < h5File.getNumDataSets(); ++i) {
        cout << "  DataSet " << i << ": " << h5File.getDataSetName(i) << endl;
    }
    string dataSetName = h5File.getDataSetName(0);
    ofxHDF5DataSetPtr dataSet = h5File.loadDataSet(dataSetName);

    int count = dataSet->getDimensionSize(0) / stride;
    dataSet->setHyperslab(0, count, stride);
    data.resize(count);

    // Data is 64-bit.
    if (bExponential) {
        // Load it in a temp double array.
        double *rawData = new double[count];
        dataSet->read(rawData);

        // Set the return array from the transformed data.
        for (int i = 0; i < count; ++i) {
            data[i] = powf(10, rawData[i]);
        }

        delete [] rawData;
    }
    else {
        // Read it directly, losing precision.
        dataSet->read(data.data(), H5_DATATYPE_FLOAT);
    }
}

//--------------------------------------------------------------
void ofApp::imGui()
{
    gui.begin();
    {
        ImGui::SetNextWindowPos(ofVec2f(10, 10));
        ImGui::SetNextWindowSize(ofVec2f(360, 140), ImGuiSetCond_Once);
        if (ImGui::Begin("VAPOR")) {
            ImGui::Text("%.1f FPS (%.3f ms/frame)", ofGetFrameRate(), 1000.0f / ImGui::GetIO().Framerate);

//            if (ImGui::CollapsingHeader("Box", nullptr, true, true)) {
//                ImGui::Checkbox("Draw Grid", &bDrawGrid);
//                if (ImGui::SliderFloat("Size", &size, 2.0f, 200.0f)) {
//                    bRebuildBox = true;
//                }
//                if (ImGui::SliderInt("Resolution", &resolution, 1, 128)) {
//                    bRebuildBox = true;
//                }
//            }
//
//            if (ImGui::CollapsingHeader("Scale", nullptr, true, true)) {
//                ImGui::Checkbox("Reset", &bReset);
//                ImGui::Checkbox("Paused", &bPaused);
//
//                ImGui::SliderFloat("Speed", &speed, 0.0f, 10.0f);
//
//                ImGui::Text("Type");
//                ImGui::RadioButton("Inflation / Dark Energy", &type, 0);
//                ImGui::RadioButton("Radiation Domination", &type, 1);
//                ImGui::RadioButton("Matter Domination", &type, 2);
//            }

            if (ImGui::CollapsingHeader("Render", nullptr, true, true)) {
                ImGui::SliderFloat("Scale", &scale, 1.0f, 2048.0f);
                ImGui::SliderFloat("Point Size", &pointSize, 0.1f, 64.0f);
                ImGui::DragFloatRange2("Density Range", &densityMin, &densityMax, 0.0001f, 0.0f, 1.0f, "Min: %.4f%%", "Max: %.4f%%");
            }
        }
        ImGui::End();
    }
    gui.end();
}

//--------------------------------------------------------------
void ofApp::update(){

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
            shader.begin();
            shader.setUniform1f("densityMin", densityMin);
            shader.setUniform1f("densityMax", densityMax);
            {
                vboMesh.draw(OF_MESH_POINTS);
            }
            shader.end();
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
