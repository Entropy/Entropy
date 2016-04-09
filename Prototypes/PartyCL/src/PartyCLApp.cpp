#include "ofxTipsyLoader.h"

#include "PartyCLApp.h"

#define USE_OPENCL 1
//#define LOAD_TIPSY 1

namespace entropy
{
    //--------------------------------------------------------------
    void PartyCLApp::setup()
    {
        ofBackground(ofColor::black);
        ofSetLogLevel(OF_LOG_VERBOSE);
        ofSetVerticalSync(false);
        ofDisableArbTex();

        // Load presets.
        presets.push_back(Preset(0.016f, 1.54f, 8.0f, 0.1f, 1.0f));
        presets.push_back(Preset(0.016f, 1.54f, 8.0f, 0.1f, 1.0f));
        presets.push_back(Preset(0.016f, 0.68f, 20.0f, 0.1f, 1.0f));
        presets.push_back(Preset(0.0006f, 0.16f, 1000.0f, 1.0f, 1.0f));
        presets.push_back(Preset(0.0006f, 0.16f, 1000.0f, 1.0f, 1.0f));
        presets.push_back(Preset(0.0019f, 0.32f, 276.0f, 1.0f, 1.0f));
        presets.push_back(Preset(0.0016f, 0.32f, 272.0f, 0.145f, 1.0f));
        presets.push_back(Preset(0.016f, 6.04f, 0.0f, 1.0f, 1.0f));

        params.setName("PartyCL");
        params.add(bPaused.set("paused", false));
        params.add(bReset.set("reset", false));
        params.add(timestep.set("timestep", 0.016, 0.0, 1.0));
        params.add(clusterScale.set("cluster scale", 1.54, 0.0, 10.0));
        params.add(velocityScale.set("velocity scale", 8.0, 4.0, 1000.0));
        params.add(softening.set("softening factor", 0.1, 0.001, 1.0));
        params.add(damping.set("velocity damping", 1.0, 0.5, 1.0));
        params.add(pointSize.set("point size", 16.0f, 1.0f, 64.0f));
        params.add(bExportFrames.set("export frames", false));
        ofAddListener(params.parameterChangedE(), this, &PartyCLApp::paramsChanged);

        camera.setDistance(32.0f);

        // Set Gui.
        guiPanel.setup(params, "partycl.xml");
        guiPanel.loadFromFile("partycl.xml");

        bGuiVisible = true;

        // Load the first preset.
        presetIndex = 0;
        loadPreset();

#ifdef LOAD_TIPSY
        filename = "galaxy_20K.bin";
#else
        filename = "";
#endif

        if (filename.length()) {
            // Clear host memory.
            hPos.clear();
            hVel.clear();
            hColor.clear();

            // Load the data file.
            int numDark;
            int numStar;
            vector<int> hIDs;
            ofxLoadTipsyFile(filename, numBodies, numDark, numStar, hPos, hVel, hIDs);
        }
        else {
            // Set the number of bodies and configure appropriately.
            numBodies = 1024 * 12;

            // Allocate host memory.
            hPos.resize(numBodies);
            hVel.resize(numBodies);
            hColor.resize(numBodies);
        }

        ofLogNotice("PartyCLApp", "Number of Bodies = %d", numBodies);

        switch (numBodies)
        {
            case 1024:
                clusterScale = 1.52f;
                velocityScale = 2.f;
                break;

            case 2048:
                clusterScale = 1.56f;
                velocityScale = 2.64f;
                break;

            case 4096:
                clusterScale = 1.68f;
                velocityScale = 2.98f;
                break;

            case 8192:
                clusterScale = 1.98f;
                velocityScale = 2.9f;
                break;

            default:
            case 16384:
                clusterScale = 1.54f;
                velocityScale = 8.f;
                break;

            case 32768:
                clusterScale = 1.44f;
                velocityScale = 11.f;
                break;
        }

        int p = 256;  // workgroup X dimension
        int q = 1;    // workgroup Y dimension

        if ((q * p) > 256) {
            p = 256 / q;
            ofLogNotice("PartyCLApp::setup", "Setting p=%d to maintain %d threads per block", p, 256);
        }

        if ((q == 1) && (numBodies < p)) {
            p = numBodies;
            ofLogNotice("PartyCLApp::setup", "Setting p=%d because # of bodies < p", p);
        }
        ofLogNotice("PartyCLApp::setup", "Workgroup Dims = (%d x %d)", p, q);

        // Init system.
#ifdef USE_OPENCL
        system = new NBodySystemOpenCL(numBodies, p, q);
#else
        system = new NBodySystemCPU(numBodies);
#endif

        // Init renderer.
        renderer = new ParticleRenderer();
        displayMode = ParticleRenderer::PARTICLE_SPRITES;

        // Start the show.
        bPaused = false;
        bReset = true;
    }

    //--------------------------------------------------------------
    void PartyCLApp::randomizeBodies()
    {
        switch (activeConfig)
        {
            default:
            case NBODY_CONFIG_RANDOM:
            {
                float scalePos = clusterScale * MAX(1.0f, numBodies / (1024.f));
                float scaleVel = velocityScale * scalePos;

                ofVec3f pos;
                ofVec3f vel;
                for (int i = 0; i < numBodies; ++i) {
                    pos.x = ofRandomf();
                    pos.y = ofRandomf();
                    pos.z = ofRandomf();
                    if (pos.lengthSquared() > 1)
                        continue;

                    vel.x = ofRandomf();
                    vel.y = ofRandomf();
                    vel.z = ofRandomf();
                    if (vel.lengthSquared() > 1)
                        continue;

                    hPos[i].x = pos.x * scalePos;
                    hPos[i].y = pos.y * scalePos;
                    hPos[i].z = pos.z * scalePos;
                    hPos[i].w = 1.0f;  // mass

                    hVel[i].x = vel.x * scaleVel; // pos.x
                    hVel[i].y = vel.y * scaleVel; // pos.x
                    hVel[i].z = vel.z * scaleVel; // pos.x
                    hVel[i].w = 1.0f;  // inverse mass
                }
            }
                break;

            case NBODY_CONFIG_SHELL:
            {
                float scalePos = clusterScale;
                float scaleVel = scalePos * velocityScale;
                float inner = 2.5f * scalePos;
                float outer = 4.0f * scalePos;

                ofVec3f pos;
                ofVec3f vel;
                ofVec3f axis;
                for (int i = 0; i < numBodies; ++i) {
                    pos.x = ofRandomf();
                    pos.y = ofRandomf();
                    pos.z = ofRandomf();
                    if (pos.lengthSquared() > 1)
                        continue;

                    hPos[i].x = pos.x * ofRandom(inner, outer);
                    hPos[i].y = pos.y * ofRandom(inner, outer);
                    hPos[i].z = pos.z * ofRandom(inner, outer);
                    hPos[i].w = 1.0f;  // mass

                    axis.set(0.0f, 0.0f, 1.0f);
                    if (1.0f - pos.dot(axis) < 1e-6) {
                        axis.x = pos.y;
                        axis.y = pos.x;
                        axis.normalize();
                    }
                    vel.set(hPos[i].x, hPos[i].y, hPos[i].z);
                    vel.cross(axis);

                    hVel[i].x = vel.x * scaleVel;
                    hVel[i].y = vel.y * scaleVel;
                    hVel[i].z = vel.z * scaleVel;
                    hVel[i].x = 1.0f;  // inverse mass
                }
            }
                break;

            case NBODY_CONFIG_EXPAND:
            {
                float scalePos = clusterScale * MAX(1.0f, numBodies / (1024.0f));
                float scaleVel = scalePos * velocityScale;

                ofVec3f point;
                for (int i = 0; i < numBodies; ++i) {
                    point.x = ofRandomf();
                    point.y = ofRandomf();
                    point.z = ofRandomf();
                    if (point.lengthSquared() > 1)
                        continue;

                    hPos[i].x = point.x * scalePos;
                    hPos[i].y = point.y * scalePos;
                    hPos[i].z = point.z * scalePos;
                    hPos[i].w = 1.0f;  // mass

                    hVel[i].x = point.x * scaleVel;
                    hVel[i].y = point.y * scaleVel;
                    hVel[i].z = point.z * scaleVel;
                    hVel[i].w = 1.0f;  // inverse mass
                }
            }
                break;
        }

//        if (color) {
//            int v = 0;
//            for(int i=0; i < numBodies; i++) {
//                //const int scale = 16;
//                color[v++] = rand() / (float) RAND_MAX;
//                color[v++] = rand() / (float) RAND_MAX;
//                color[v++] = rand() / (float) RAND_MAX;
//                color[v++] = 1.0f;
//            }
//        }
    }

    //--------------------------------------------------------------
    void PartyCLApp::resetSimulation()
    {
        ofLogNotice("PartyCLApp::resetSim", "Resetting Nbody system...");

        if (filename.length() == 0) {
            randomizeBodies();
        }
        else {
            damping = 1.0;
            softening = 0.1;
            timestep = 0.016;
            clusterScale = 1.56;
            velocityScale = 2.64;
        }

        system->setArray(NBodySystem::ARRAY_POSITION, (float *)hPos.data());
        system->setArray(NBodySystem::ARRAY_VELOCITY, (float *)hVel.data());

//        renderer->setColors(hColor, numBodies);
    }

    //--------------------------------------------------------------
    void PartyCLApp::loadPreset()
    {
        timestep = presets[presetIndex].timestep;
        clusterScale = presets[presetIndex].clusterScale;
        velocityScale = presets[presetIndex].velocityScale;
        softening = presets[presetIndex].softening;
        damping = presets[presetIndex].damping;
    }

    //--------------------------------------------------------------
    void PartyCLApp::update()
    {
        ofSetWindowTitle(ofToString(ofGetFrameRate(), 2) + " FPS");

        if (bReset) {
            resetSimulation();
            bReset = false;
        }

        if (!bPaused) {
            // Set simulation parameters.
            system->setSoftening(softening);
            system->setDamping(damping);

            // Run the simulation computations.
            system->update(timestep);

            // Set renderer parameters.
            renderer->setPointSize(pointSize);
        }
    }


    //--------------------------------------------------------------
    void PartyCLApp::draw()
    {
        camera.begin();
        renderer->display(system->getVbo(), system->getNumBodies(), displayMode);

//        ofDrawAxis(10);

        camera.end();

        if (bGuiVisible) {
            guiPanel.draw();
        }
    }

    //--------------------------------------------------------------
    void PartyCLApp::keyPressed(int key)
    {
        switch (key)
        {
            case ' ':
                bPaused ^= 1;
                break;

            case '`':
                bGuiVisible ^= 1;
                break;

            case 'p':
            case 'P':
                displayMode = (ParticleRenderer::DisplayMode)((displayMode + 1) % ParticleRenderer::PARTICLE_NUM_MODES);
                break;

            case 'r':
            case 'R':
                bReset = true;
                break;

            case '[':
                presetIndex = (presetIndex == 0) ? presets.size() - 1 : (presetIndex - 1) % presets.size();
                loadPreset();
                resetSimulation();
                break;

            case ']':
                presetIndex = (presetIndex + 1) % presets.size();
                loadPreset();
                resetSimulation();
                break;

            case 'f':
            case 'F':
                ofToggleFullscreen();
                break;

            case '1':
                activeConfig = NBODY_CONFIG_SHELL;
                resetSimulation();
                break;

            case '2':
                activeConfig = NBODY_CONFIG_RANDOM;
                resetSimulation();
                break;

            case '3':
                activeConfig = NBODY_CONFIG_EXPAND;
                resetSimulation();
                break;
        }
    }

    //--------------------------------------------------------------
    void PartyCLApp::paramsChanged(ofAbstractParameter& param)
    {
        string paramName = param.getName();

        if (paramName == clusterScale.getName() ||
            paramName == velocityScale.getName()) {
            bReset = true;
        }
    }
}
