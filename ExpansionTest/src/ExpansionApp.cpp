#include "ExpansionApp.h"

namespace entropy
{
    //--------------------------------------------------------------
    void ExpansionParticle::setup(float dim)
    {
        float range = dim * 0.5;
        ini = ofVec3f(ofRandomf() * range, ofRandomf() * range, ofRandomf() * range);

        type = ofRandom(4) + 1;
        size = ofRandom(16, 64);
    }

    //--------------------------------------------------------------
    void ExpansionParticle::update(float scale)
    {
        pos = ini * scale;
    }

    //--------------------------------------------------------------
    void ExpansionApp::setup()
    {
        ofSetLogLevel(OF_LOG_VERBOSE);
        ofDisableArbTex();
        ofBackground(ofColor::black);

        shader.load("shaders/billboard");

        ofDirectory dir = ofDirectory("textures");
        dir.listDir();
        textures.resize(dir.size());
        for (int i = 0; i < dir.size(); ++i) {
            textures[i].load(dir.getFile(i));
        }

        easyCam.setNearClip(0.0);
        easyCam.setFarClip(FLT_MAX);
        camera.setup();

        paramGroup.setName("EXPANSION");
        paramGroup.add(size.set("SIZE", 10.0, 1.0, 100.0));
        paramGroup.add(resolution.set("RESOLUTION", 2, 1, 128));
        paramGroup.add(speed.set("SPEED ", 0.25, 0.0, 1.0));
        paramGroup.add(numParticles.set("NUM PARTICLES", 512, 8, 2048));
        paramGroup.add(bDrawGrid.set("DRAW GRID", false));
        paramGroup.add(bDebugCamera.set("DEBUG CAMERA", false));
        paramGroup.add(bRestart.set("RESTART", true));

        guiPanel.setup(paramGroup, "expansion.xml");
        guiPanel.loadFromFile("expansion.xml");

        bRestart = true;
        bGuiVisible = true;
    }

    //--------------------------------------------------------------
    void ExpansionApp::restart()
    {
        box.set(size, size, size, resolution, resolution, resolution);
        scale = 1.0;

        // Add particles.
        particles.resize(numParticles);
        for (int i = 0; i < particles.size(); ++i) {
            particles[i].setup(size);
        }

        // Set the first particle as the Camera.
        particles[0].type = ExpansionParticle::TypeCamera;
//        particles[0].ini = ofVec3f(size * 0.5, size * 0.5, size * 0.5);

        // Setup the VBO mesh.
        vboMesh.clear();
        vboMesh.getVertices().resize(particles.size());
        vboMesh.getNormals().resize(particles.size());
        vboMesh.setUsage(GL_DYNAMIC_DRAW);
        vboMesh.setMode(OF_PRIMITIVE_POINTS);

        // Upload the normal data (x = size, y = type).
        for (int i = 0; i < particles.size(); ++i) {
            vboMesh.getNormals()[i].set(ofVec3f(particles[i].size, particles[i].type));
            cout << i << ": Normal " << vboMesh.getNormals()[i] << endl;
        }

        bRestart = false;
    }

    //--------------------------------------------------------------
    void ExpansionApp::update()
    {
        if (bRestart) {
            restart();
        }

        // Ignore presses over the GUI.
//        bool bMousePressed = ofGetMousePressed() && (!bGuiVisible || !guiPanel.getShape().inside(ofGetMouseX(), ofGetMouseY()));

        // Update the scale.
        scale += exp(ofGetLastFrameTime()) * speed;
        ofLogVerbose() << "Scale is " << scale;

        // Update all particles.
        for (int i = 0; i < particles.size(); ++i) {
            particles[i].update(scale);
            vboMesh.getVertices()[i].set(particles[i].pos);
        }

        // Update camera position.
        camera.setPosition(particles[0].pos);
    }

    //--------------------------------------------------------------
    void ExpansionApp::draw()
    {
        ofSetColor(ofColor::white);

        if (bDebugCamera) {
            easyCam.begin();
        }
        else {
            camera.begin();
        }
        {
            if (bDrawGrid) {
                // Draw the universe.
                ofPushMatrix();
                ofScale(scale, scale, scale);
                {
                    box.draw(OF_MESH_WIREFRAME);
                }
                ofPopMatrix();
            }

            // Draw all particles.
            ofEnableBlendMode(OF_BLENDMODE_SCREEN);

            shader.begin();
            for (int i = 0; i < textures.size(); ++i) {
                shader.setUniformTexture("texture" + ofToString(i), textures[i], i + 1);
            }

//            shader.printActiveUniforms();

            ofEnablePointSprites();
            ofSetColor(ofColor::white);
            vboMesh.draw();
            ofDisablePointSprites();

            shader.end();

            ofEnableBlendMode(OF_BLENDMODE_ALPHA);


            if (bDebugCamera) {
                ofSetColor(ofColor::red);
                ofDrawArrow(camera.getGlobalPosition(), camera.getGlobalPosition() + camera.getOrientationQuat() * camera.getLookAtDir() * size);
            }

            ofDrawAxis(size);
        }
        if (bDebugCamera) {
            easyCam.end();
        }
        else {
            camera.end();
        }

        if (bGuiVisible) {
            guiPanel.draw();
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
        bRestart = true;
    }

    //--------------------------------------------------------------
    void ExpansionApp::gotMessage(ofMessage msg){
        
    }
    
    //--------------------------------------------------------------
    void ExpansionApp::dragEvent(ofDragInfo dragInfo){ 
        
    }
}
