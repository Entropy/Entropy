#include "ofApp.h"
#include "H5Cpp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
    ofBackground(ofColor::black);
    ofSetLogLevel(OF_LOG_NOTICE);

    ofDirectory dir("subbox");
    dir.listDir();
//    for (int i = 0; i < dir.size(); ++i) {
//        cout << i << ": " << dir.getName(i) << endl;
//    }

    string groupName = "PartType1";
    string dataSetIDsName = "ParticleIDs";
    string dataSetCoordsName = "Coordinates";

    ofxHDF5File h5File;
    ofxHDF5GroupPtr h5Goup;
    ofxHDF5DataSetPtr h5DataSetIDs;
    ofxHDF5DataSetPtr h5DataSetCoords;

    vector<int> ids;

    // First pass, get ranges.
    meshes.resize(dir.size());
    int minCount = INT_MAX;
    int maxCount = INT_MIN;
    int minID = INT_MAX;
    int maxID = INT_MIN;
    minCoord.set(FLT_MAX, FLT_MAX, FLT_MAX);
    maxCoord.set(FLT_MIN, FLT_MIN, FLT_MIN);
    for (int i = 0; i < dir.size(); ++i) {
        cout << "Processing file " << dir.getName(i) << "... " << endl;
        string file = dir.getPath(i);
        h5File.open(file, true);
        h5Goup = h5File.loadGroup(groupName);
//        h5DataSetIDs = h5Goup->loadDataSet(dataSetIDsName);
//        h5DataSetCoords = h5Goup->loadDataSet(dataSetCoordsName);
//
//        int offset = 0;
//        int count = min(h5DataSetIDs->getDimensionSize(0), h5DataSetCoords->getDimensionSize(0));
//        int stride = 1;
//
//        minCount = min(minCount, count);
//        maxCount = max(maxCount, count);
//
//        ids.resize(count);
//        h5DataSetIDs->setHyperslab(offset, count, stride);
//        h5DataSetIDs->read(ids.data());
//        int currMinID = *std::min_element(ids.begin(), ids.end());
//        int currMaxID = *std::max_element(ids.begin(), ids.end());
//        minID = min(minID, currMinID);
//        maxID = max(maxID, currMaxID);
//
//        coords.resize(count * 3);
//        h5DataSetCoords->setHyperslab(offset, count, stride);
//        h5DataSetCoords->read(coords.data());
//
//        float currMinCoord = *std::min_element(coords.begin(), coords.end());
//        float currMaxCoord = *std::max_element(coords.begin(), coords.end());
//        minCoord = min(minCoord, currMinCoord);
//        maxCoord = max(maxCoord, currMaxCoord);

        h5DataSetCoords = h5Goup->loadDataSet(dataSetCoordsName);

        int offset = 0;
        int count = h5DataSetCoords->getDimensionSize(0);
        int stride = 1;

        meshes[i].getVertices().resize(count);
        h5DataSetCoords->setHyperslab(offset, count, stride);
        h5DataSetCoords->read(meshes[i].getVerticesPointer());

        for (ofVec3f v : meshes[i].getVertices()) {
            minCoord.x = min(minCoord.x, v.x);
            minCoord.y = min(minCoord.y, v.y);
            minCoord.z = min(minCoord.z, v.z);

            maxCoord.x = max(maxCoord.x, v.x);
            maxCoord.y = max(maxCoord.y, v.y);
            maxCoord.z = max(maxCoord.z, v.z);
        }
    }

//    cout << "Counts in range [" << minCount << ", " << maxCount << "]" << endl;
//    cout << "IDs in range [" << minID << ", " << maxID << "]" << endl;
    cout << "Coords in range (" << minCoord << ") to (" << maxCoord << ")" << endl;


//    vector<string> filenames;
//    filenames.push_back("snap_subbox3_000.hdf5");
//    filenames.push_back("snap_subbox3_050.hdf5");
//    filenames.push_back("snap_subbox3_100.hdf5");
//    filenames.push_back("snap_subbox3_150.hdf5");
////    string filename = "snap_subbox3_000.hdf5";
//    string dataSetName = "Coordinates";
//
//    int total = 0;
//    for (auto& name : filenames) {
//        hdf5File.open(name, true);
//        ofxHDF5GroupPtr group = hdf5File.loadGroup(groupName);
//        ofxHDF5DataSetPtr dataSet = group->loadDataSet(dataSetName);
//
//        int offset = 0;
//        int count = dataSet->getDimensionSize(0);
//        int stride = 1;
//        dataSet->setHyperslab(offset, count, stride);
//
//        mesh.getVertices().resize(total + count);
//        dataSet->read(&mesh.getVertices()[total]);
//
//        total += count;
//    }

//    for (int i = 0; i < mesh.getNumVertices(); ++i) {
//        cout << i << "[" << (offset + i * stride) << "] => " << mesh.getVertex(i) << endl;
//    }
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw()
{
    ofSetColor(ofColor::white);

    cam.setNearClip(0);
    cam.setFarClip(FLT_MAX);

    int idx = ofGetFrameNum() % meshes.size();
    ofVec3f translate = (maxCoord - minCoord) * -0.5 - minCoord;
    float scale = 1.0;

    cam.begin();
    {
        ofPushMatrix();
        ofTranslate(translate);
        ofScale(scale, scale, scale);
        {
            meshes[idx].draw(OF_MESH_POINTS);
        }
        ofPopMatrix();

        ofDrawAxis(20);
    }
    cam.end();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

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
