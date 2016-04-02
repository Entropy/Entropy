#include "CMBApp.h"
#include "GLError.h"

namespace entropy
{
    //--------------------------------------------------------------
    void CMBApp::setup()
    {
        ofSetLogLevel(OF_LOG_VERBOSE);
        //ofSetVerticalSync(false);

        tintColor = ofColor::white;
        dropColor = ofColor::red;

        bDropOnPress = false;
        bDropUnderMouse = false;
        dropRate = 1;

        damping = 0.995f;
        radius = 10.0f;
        ringSize = 0.5f;

        bRestart = true;
        bGuiVisible = true;

#ifdef COMPUTE_OPENCL
        openCL.setupFromOpenGL();

        openCL.loadProgramFromFile("cl/ripples.cl");
#ifdef THREE_D
        dropKernel = openCL.loadKernel("drop3D");
        ripplesKernel = openCL.loadKernel("ripples3D");
        copyKernel = openCL.loadKernel("copy3D");
#else
        dropKernel = openCL.loadKernel("drop2D");
        ripplesKernel = openCL.loadKernel("ripples2D");
        copyKernel = openCL.loadKernel("copy2D");
#endif  // THREE_D
#endif  // COMPUTE_OPENCL

#ifdef COMPUTE_GLSL
#ifdef THREE_D
		dropShader.load("shaders/passthru.vert", "shaders/drop3D.frag", "shaders/layer.geom");
		ripplesShader.load("shaders/passthru.vert", "shaders/ripples3D.frag", "shaders/layer.geom");
		copyShader.load("shaders/passthru.vert", "shaders/copy3D.frag", "shaders/layer.geom");

		cam.setDistance(1000);
#else
        ripplesShader.load("shaders/passthru.vert", "shaders/ripples.frag");
#endif  // THREE_D
#endif
    }

    //--------------------------------------------------------------
    void CMBApp::restart()
    {
        activeIndex = 0;

#ifdef THREE_D
        dimensions.x = 128;
		dimensions.y = 128;
        dimensions.z = 128;
#else
        dimensions.x = ofGetWidth();
        dimensions.y = ofGetHeight();

#ifdef COMPUTE_GLSL
#endif  // COMPUTE_GLSL

#endif  // THREE_D

        for (int i = 0; i < 3; ++i) {
#ifdef COMPUTE_OPENCL
#ifdef THREE_D
            clImages[i].initWithTexture3D(dimensions.x, dimensions.y, dimensions.z, GL_RGBA32F);
#else
            clImages[i].initWithTexture(dimensions.x, dimensions.y, GL_RGBA32F);
#endif  // THREE_D
#endif  // COMPUTE_OPENCL

#ifdef COMPUTE_GLSL
#ifdef THREE_D
            textures[i].allocate(dimensions.x, dimensions.y, dimensions.z, GL_RGBA32F);
#else
			textures[i].allocate(dimensions.x, dimensions.y, GL_RGBA32F);
#endif

            fbos[i].allocate();
            fbos[i].attachTexture(textures[i], 0);
            fbos[i].begin();
            {
                ofClear(0, 0);
            }
            fbos[i].end();
            fbos[i].checkStatus();
#endif  // COMPUTE_GLSL
        }

#ifdef COMPUTE_GLSL
#ifdef THREE_D
		ofDefaultVec3 origin = ofDefaultVec3(0.0, ofGetHeight() - dimensions.y);

		mesh.setMode(OF_PRIMITIVE_TRIANGLES);
		mesh.addVertex(origin + ofDefaultVec3(0.0, 0.0, 0.0));
		mesh.addVertex(origin + ofDefaultVec3(dimensions.x, 0.0, 0.0));
		mesh.addVertex(origin + ofDefaultVec3(0.0, dimensions.y, 0.0));

		mesh.addColor(ofFloatColor(1.0, 0.0, 0.0));
		mesh.addColor(ofFloatColor(0.0, 1.0, 0.0));
		mesh.addColor(ofFloatColor(0.0, 0.0, 1.0));

		mesh.addTexCoord(ofVec2f(0.0, 0.0));
		mesh.addTexCoord(ofVec2f(dimensions.x, 0.0));
		mesh.addTexCoord(ofVec2f(0.0, dimensions.y));

		mesh.addVertex(origin + ofVec3f(dimensions.x, 0.0, 0.0));
		mesh.addVertex(origin + ofVec3f(0.0, dimensions.y, 0.0));
		mesh.addVertex(origin + ofVec3f(dimensions.x, dimensions.y, 0.0));

		mesh.addColor(ofFloatColor(0.0, 1.0, 0.0));
		mesh.addColor(ofFloatColor(0.0, 0.0, 1.0));
		mesh.addColor(ofFloatColor(1.0, 1.0, 0.0));

		mesh.addTexCoord(ofVec2f(dimensions.x, 0.0));
		mesh.addTexCoord(ofVec2f(0.0, dimensions.y));
		mesh.addTexCoord(ofVec2f(dimensions.x, dimensions.y));
#else
        // Build a mesh to render a quad.
        mesh.clear();
        mesh.setMode(OF_PRIMITIVE_TRIANGLE_FAN);

        mesh.addVertex(ofVec3f(0, 0));
        mesh.addVertex(ofVec3f(dimensions.x, 0));
        mesh.addVertex(ofVec3f(dimensions.x, dimensions.y));
        mesh.addVertex(ofVec3f(0, dimensions.y));

        mesh.addTexCoord(ofVec2f(0, 0));
        mesh.addTexCoord(ofVec2f(dimensions.x, 0));
        mesh.addTexCoord(ofVec2f(dimensions.x, dimensions.y));
        mesh.addTexCoord(ofVec2f(0, dimensions.y));
#endif
#endif  // COMPUTE_GLSL

        bRestart = false;
    }

    //--------------------------------------------------------------
	void CMBApp::update()
	{
		if (bRestart) {// || dimensions.x != ofGetWidth() || dimensions.y != ofGetHeight()) {
			restart();
		}

		int srcIdx = (activeIndex + 1) % 2;
		int dstIdx = activeIndex;

		// Add new drops.
		bool bMousePressed = ofGetMousePressed() && !bMouseOverGui;
		if ((bDropOnPress && bMousePressed) || (!bDropOnPress && ofGetFrameNum() % dropRate == 0)) {
#ifdef COMPUTE_OPENCL
			dropKernel->setArg(0, clImages[srcIdx]);
#ifdef THREE_D
			dropKernel->setArg(1, bDropUnderMouse ? ofVec4f(ofGetMouseX(), ofGetMouseY(), ofGetMouseY(), 0) : ofVec4f(ofRandom(dimensions.x), ofRandom(dimensions.y), ofRandom(dimensions.z), 0));
#else
			dropKernel->setArg(1, bDropUnderMouse ? ofVec2f(ofGetMouseX(), ofGetMouseY()) : ofVec2f(ofRandom(dimensions.x), ofRandom(dimensions.y)));
#endif  // THREE_D

			dropKernel->setArg(2, radius);
			dropKernel->setArg(3, ringSize);
			dropKernel->setArg(4, dropColor);
#ifdef THREE_D
			dropKernel->run3D(dimensions.x, dimensions.y, dimensions.z);
#else
			dropKernel->run2D(dimensions.x, dimensions.y);
#endif  // THREE_D

			openCL.finish();
#endif  // COMPUTE_OPENCL

#ifdef COMPUTE_GLSL
#ifdef THREE_D
			fbos[srcIdx].begin();

			ofDefaultVec3 burstPos = ofDefaultVec3(ofRandom(dimensions.x), ofRandom(dimensions.y), ofRandom(dimensions.z));
			float burstThickness = 1.0f;

			dropShader.begin();
			dropShader.setUniform3f("uBurst.pos", burstPos);
			dropShader.setUniform1f("uBurst.radius", radius);
			dropShader.setUniform1f("uBurst.thickness", burstThickness);

			//dropShader.printActiveUniforms();

			ofSetColor(255, 0, 0);

			int minLayer = MAX(0, burstPos.z - radius - burstThickness);
			int maxLayer = MIN(volumetrics.getVolumeDepth() - 1, burstPos.z + radius + burstThickness);
			for (int i = minLayer; i <= maxLayer; ++i) {
				//for (int i = 0; i < volumetrics.getVolumeDepth(); ++i) {
				dropShader.setUniform1i("uLayer", i);
				mesh.draw();
			}

			dropShader.end();
			fbos[srcIdx].end();
#else
			ofPushStyle();
			ofPushMatrix();

			fbos[srcIdx].begin();
			ofScale(1.0, -1.0, 1.0);
			ofTranslate(0.0, -ofGetHeight(), 0.0);
			{
				ofSetColor(dropColor);
				ofNoFill();
				if (bDropUnderMouse) {
					ofDrawCircle(ofGetMouseX(), ofGetMouseY(), radius);
				}
				else {
					ofDrawCircle(ofRandomWidth(), ofRandomHeight(), radius);
				}
			}
			fbos[srcIdx].end();

			ofPopMatrix();
			ofPopStyle();
#endif  // THREE_D
#endif  // COMPUTE_GLSL
		}

		// Layer the drops.
#ifdef COMPUTE_OPENCL
		ripplesKernel->setArg(0, clImages[srcIdx]);
		ripplesKernel->setArg(1, clImages[dstIdx]);
		ripplesKernel->setArg(2, clImages[2]);
		ripplesKernel->setArg(3, damping / 10.0f + 0.9f);  // 0.9 - 1.0 range
#ifdef THREE_D
		ripplesKernel->run3D(dimensions.x, dimensions.y, dimensions.z);
#else
		ripplesKernel->run2D(dimensions.x, dimensions.y);
#endif  // THREE_D

		openCL.finish();

		// Copy temp image to destination (necessary since we can't read_write in OpenCL 1.2)
		copyKernel->setArg(0, clImages[2]);
		copyKernel->setArg(1, clImages[dstIdx]);
#ifdef THREE_D
		copyKernel->run3D(dimensions.x, dimensions.y, dimensions.z);

		openCL.finish();

		volumetrics.setup(&clImages[dstIdx].getTexture3D(), ofVec3f(1, 1, 1));
#else
		copyKernel->run2D(dimensions.x, dimensions.y);
#endif  // THREE_D
#endif  // COMPUTE_OPENCL

#ifdef COMPUTE_GLSL
		// Layer the drops.
		fbos[2].begin();
		ripplesShader.begin();
		ripplesShader.setUniform1f("uDamping", damping / 10.0f + 0.9f);  // 0.9 - 1.0 range
#ifdef THREE_D
		ripplesShader.setUniformTexture("uPrevBuffer", textures[dstIdx].texData.textureTarget, textures[dstIdx].texData.textureID, 1);
		ripplesShader.setUniformTexture("uCurrBuffer", textures[srcIdx].texData.textureTarget, textures[srcIdx].texData.textureID, 2);
		ripplesShader.setUniform3f("uDims", dimensions);
		for (int i = 0; i < volumetrics.getVolumeDepth(); ++i)
		{
			ripplesShader.setUniform1i("uLayer", i);
			mesh.draw();
		}
#else
		ripplesShader.setUniformTexture("uPrevBuffer", textures[dstIdx], 1);
		ripplesShader.setUniformTexture("uCurrBuffer", textures[srcIdx], 2);
		{
			mesh.draw();
		}
#endif  // THREE_D
		ripplesShader.end();
		fbos[2].end();

		// Copy temp image to destination.
		fbos[dstIdx].begin();
		{
#ifdef THREE_D
			copyShader.begin();
			copyShader.setUniformTexture("uCopyBuffer", textures[2].texData.textureTarget, textures[2].texData.textureID, 1);
			copyShader.setUniform3f("uDims", dimensions);
			{
				for (int i = 0; i < volumetrics.getVolumeDepth(); ++i)
				{
					copyShader.setUniform1i("uLayer", i);
					mesh.draw();
				}
			}
			copyShader.end();
#else
			textures[2].bind();
			mesh.draw();
			textures[2].unbind();
#endif  // THREE_D
		}
		fbos[dstIdx].end();

#ifdef THREE_D
		volumetrics.setup(&textures[dstIdx], ofVec3f(1, 1, 1));
#endif  // THREE_D
#endif  // COMPUTE_GLSL

#ifdef THREE_D
        volumetrics.setRenderSettings(1.0, 1.0, 1.0, 0.1);
#endif  // THREE_D

        activeIndex = 1 - activeIndex;
    }

    //--------------------------------------------------------------
    void CMBApp::imGui()
    {
        static const int kGuiMargin = 10;

        gui.begin();
        {
            ofVec2f windowPos(kGuiMargin, kGuiMargin);
            ofVec2f windowSize = ofVec2f::zero();

            ImGui::SetNextWindowPos(windowPos, ImGuiSetCond_Appearing);
            ImGui::SetNextWindowSize(ofVec2f(380, 94), ImGuiSetCond_Appearing);
            if (ImGui::Begin("CMB", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("%.1f FPS (%.3f ms/frame)", ofGetFrameRate(), 1000.0f / ImGui::GetIO().Framerate);

                ImGui::Checkbox("Restart", &bRestart);

                ImGui::ColorEdit3("Tint Color", &tintColor[0]);
                ImGui::ColorEdit3("Drop Color", &dropColor[0]);

                ImGui::Checkbox("Drop On Press", &bDropOnPress);
                ImGui::Checkbox("Drop Under Mouse", &bDropUnderMouse);

                ImGui::SliderInt("Drop Rate", &dropRate, 1, 60);
                ImGui::SliderFloat("Damping", &damping, 0.0f, 1.0f);
                ImGui::SliderFloat("Radius", &radius, 1.0f, 50.0f);
                ImGui::SliderFloat("Ring Size", &ringSize, 0.0f, 5.0f);

                windowSize.set(ImGui::GetWindowSize());
                ImGui::End();
            }

            ofRectangle windowBounds(windowPos, windowSize.x, windowSize.y);
            bMouseOverGui = windowBounds.inside(ofGetMouseX(), ofGetMouseY());
        }
        gui.end();
    }

    //--------------------------------------------------------------
    void CMBApp::draw()
    {
        ofBackground(255);

        ofPushStyle();
        ofSetColor(tintColor);

		ofEnableAlphaBlending();
		ofDisableDepthTest();

#ifdef THREE_D
        cam.begin();
        {
            volumetrics.drawVolume(0, 0, 0, ofGetHeight(), 0);
        }
        cam.end();
#else
        int drawIdx = 1 - activeIndex;

#ifdef COMPUTE_OPENCL
        clImages[drawIdx].getTexture().draw(0, 0);
#endif  // COMPUTE_OPENCL

#ifdef COMPUTE_GLSL
        textures[drawIdx].draw(0, 0);
#endif  // COMPUTE_GLSL

#endif  // THREE_D

        ofPopStyle();

        if (bGuiVisible) {
            imGui();
        }
    }

    //--------------------------------------------------------------
    void CMBApp::keyPressed(int key)
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
    void CMBApp::keyReleased(int key){

    }

    //--------------------------------------------------------------
    void CMBApp::mouseMoved(int x, int y ){

    }

    //--------------------------------------------------------------
    void CMBApp::mouseDragged(int x, int y, int button){

    }

    //--------------------------------------------------------------
    void CMBApp::mousePressed(int x, int y, int button){

    }

    //--------------------------------------------------------------
    void CMBApp::mouseReleased(int x, int y, int button){

    }

    //--------------------------------------------------------------
    void CMBApp::mouseEntered(int x, int y){

    }

    //--------------------------------------------------------------
    void CMBApp::mouseExited(int x, int y){

    }

    //--------------------------------------------------------------
    void CMBApp::windowResized(int w, int h)
    {
        bRestart = true;
    }

    //--------------------------------------------------------------
    void CMBApp::gotMessage(ofMessage msg){
        
    }
    
    //--------------------------------------------------------------
    void CMBApp::dragEvent(ofDragInfo dragInfo){ 
        
    }
}
