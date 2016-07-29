#include "InflationApp.h"
#include "Constants.h"

namespace entropy
{
    //--------------------------------------------------------------
    void InflationApp::setup()
    {
        // Marching Cubes
        gpuMarchingCubes.setup();

        panelMarchingCubes.loadFromFile("marching-cubes.json");
        panelMarchingCubes.setPosition(0, 0);


        // Noise Field
        noiseField.setup(gpuMarchingCubes.resolution);

        panelNoiseField.loadFromFile("noise-field.json");
        panelNoiseField.setPosition(0, panelMarchingCubes.getShape().getMaxY() + 1);

        // Render
        panelRender.loadFromFile("render.json");
        panelRender.setPosition(0, panelNoiseField.getShape().getMaxY() + 1);
        record.setSerializable(false);

        camera.setDistance(2);
        camera.setNearClip(0.1);
        camera.setFarClip(100000);

		guiVisible = true;
		ofSetBackgroundColor(0);
        now = 0;

        // postpo and recording
        ofFbo::Settings settings;
        settings.width = ofGetWidth();
        settings.height = ofGetHeight();
        settings.textureTarget = GL_TEXTURE_2D;
        settings.internalformat = GL_RGBA32F;

        fboscene.allocate(settings);
        fbobright.allocate(settings);
        fbo2.allocate(settings);
        settings.internalformat = GL_RGB;
        finalFbo.allocate(settings);
        shaderBright.load("shaders/vert_full_quad.glsl", "shaders/frag_bright.glsl");

        ofFile frag_blur("shaders/frag_blur.glsl");
        auto frag_blur_src = ofBuffer(frag_blur);
        ofFile vert_blur("shaders/vert_blur.glsl");
        auto vert_blur_src = ofBuffer(vert_blur);

        blurV.setupShaderFromSource(GL_VERTEX_SHADER,
            "#version 330\n#define PASS_V\n#define BLUR9\n" +
            vert_blur_src.getText());
        blurV.setupShaderFromSource(GL_FRAGMENT_SHADER,
              "#version 330\n#define PASS_V\n#define BLUR9\n" +
              frag_blur_src.getText());
        blurV.bindDefaults();
        blurV.linkProgram();

        blurH.setupShaderFromSource(GL_VERTEX_SHADER,
            "#version 330\n#define PASS_H\n#define BLUR9\n" +
            vert_blur_src.getText());
        blurH.setupShaderFromSource(GL_FRAGMENT_SHADER,
              "#version 330\n#define PASS_H\n#define BLUR9\n" +
              frag_blur_src.getText());
        blurH.bindDefaults();
        blurH.linkProgram();

        tonemap.load("shaders/vert_full_quad.glsl", "shaders/frag_tonemap.glsl");
        saverThread.setup(ofGetWidth(), ofGetHeight(), OF_PIXELS_RGB, OF_IMAGE_FORMAT_JPEG);

    }

    //--------------------------------------------------------------
    void InflationApp::update()
    {
		if(simulationRunning){
            now += ofGetElapsedTimef();
            noiseField.update(inflation);
            if(inflation){
                scale += ofGetElapsedTimef() * 0.1;
            }
		}
    }

    float gaussian(float x, float mu, float sigma){
        auto d = x - mu;
        auto n = 1.0 / (sqrtf(glm::two_pi<float>()) * sigma);
        return exp(-d * d/(2.0 * sigma * sigma)) * n;
    }

    //--------------------------------------------------------------
    void InflationApp::draw()
	{

        if(debug){
            camera.begin();
            noiseField.draw(threshold);
            camera.end();
        }else{
            if(additiveBlending){
                ofEnableBlendMode(OF_BLENDMODE_ADD);
            }else{
                ofEnableBlendMode(OF_BLENDMODE_ALPHA);
            }
            fboscene.begin(false);
            if(gpuMarchingCubes.shadeNormals){
                ofEnableDepthTest();
            }
            ofClear(0,255);

            camera.begin();
            ofScale(scale,scale,scale);

            gpuMarchingCubes.draw(noiseField.getTexture(), threshold);

            camera.end();
            if(gpuMarchingCubes.shadeNormals){
                ofDisableDepthTest();
            }
            fboscene.end();

            if(!bloom){
                fboscene.draw(0,0);
            }else{
                auto & texture = fboscene.getTexture();
                auto texel_size = glm::vec2(1./float(texture.getWidth()), 1./float(texture.getHeight()));

                auto w0 = gaussian(0.0, 0.0, sigma);
                auto w1 = gaussian(1.0, 0.0, sigma);
                auto w2 = gaussian(2.0, 0.0, sigma);
                auto w3 = gaussian(3.0, 0.0, sigma);
                auto w4 = gaussian(4.0, 0.0, sigma);
                auto w5 = gaussian(5.0, 0.0, sigma);
                auto w6 = gaussian(6.0, 0.0, sigma);
                auto w7 = gaussian(7.0, 0.0, sigma);
                auto w8 = gaussian(8.0, 0.0, sigma);
                auto wn = w0 + 2.0 * (w1 + w2 + w3 + w4 + w5 + w6 + w7 + w8);

                ofMesh fullQuad;
                fullQuad.addVertices({{-1, -1, 0}, { -1,1,0}, {1,1,0}, {1,-1,0}});
                fullQuad.addTexCoords({{0,1},{0,0},{1,0},{1,1}});
                fullQuad.setMode(OF_PRIMITIVE_TRIANGLE_FAN);

                fbobright.begin();
                ofClear(0,255);
                shaderBright.begin();
                blurV.setUniformTexture("tex0", texture, 0);
                shaderBright.setUniform1f("bright_threshold", brightThres);
                fullQuad.draw();
                shaderBright.end();
                fbobright.end();

                fbo2.begin();
                ofClear(0,255);
                blurV.begin();
                blurV.setUniformTexture("tex0", fbobright.getTexture(), 0);
                blurV.setUniform2f("texel_size", texel_size);
                blurV.setUniform1f("w0", w0/wn);
                blurV.setUniform1f("w1", w1/wn);
                blurV.setUniform1f("w2", w2/wn);
                blurV.setUniform1f("w3", w3/wn);
                blurV.setUniform1f("w4", w4/wn);
                blurV.setUniform1f("w5", w5/wn);
                blurV.setUniform1f("w6", w6/wn);
                blurV.setUniform1f("w7", w7/wn);
                blurV.setUniform1f("w8", w8/wn);
                fullQuad.draw();
                blurV.end();
                fbo2.end();

                fbobright.begin();
                ofClear(0,255);
                blurH.begin();
                blurH.setUniformTexture("tex0", fbo2.getTexture(), 0);
                blurH.setUniform2f("texel_size", texel_size);
                blurH.setUniform1f("w0", w0/wn);
                blurH.setUniform1f("w1", w1/wn);
                blurH.setUniform1f("w2", w2/wn);
                blurH.setUniform1f("w3", w3/wn);
                blurH.setUniform1f("w4", w4/wn);
                blurH.setUniform1f("w5", w5/wn);
                blurH.setUniform1f("w6", w6/wn);
                blurH.setUniform1f("w7", w7/wn);
                blurH.setUniform1f("w8", w8/wn);
                fullQuad.draw();
                blurH.end();
                fbobright.end();

                //bobright.draw(0,0);

                finalFbo.begin();
                ofClear(0,255);
                tonemap.begin();
                tonemap.setUniformTexture("tex0",texture, 0);
                tonemap.setUniformTexture("blurred1",fbobright.getTexture(), 1);
                tonemap.setUniform1f("contrast",contrast);
                tonemap.setUniform1f("brightness",brightness);
                tonemap.setUniform1f("tonemap_type",tonemapType);
                fullQuad.draw();
                tonemap.end();
                finalFbo.end();

                if(record){
                    saverThread.save(finalFbo.getTexture());
                }

                finalFbo.draw(0,0);
            }
            ofEnableBlendMode(OF_BLENDMODE_ALPHA);
        }

        if (guiVisible) {
            panelMarchingCubes.draw();
            panelNoiseField.draw();
            panelRender.draw();
        }

		ofDrawBitmapString(ofGetFrameRate(), ofGetWidth() - 100, 20);

		ofDrawBitmapString(timeToSetIso, ofGetWidth() - 100, 40);
		ofDrawBitmapString(timeToUpdate, ofGetWidth() - 100, 60);
    }

    //--------------------------------------------------------------
	void InflationApp::keyPressed(int key){
        switch (key) {
			case 'h':
				guiVisible = !guiVisible;
                break;

			case ' ':
				simulationRunning = !simulationRunning;
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

}
