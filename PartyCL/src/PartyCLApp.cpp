/******

 This example updates 1M particles on the GPU using OpenCL
 The OpenCL kernel writes position data directly to a VBO stored in the OpenGL device memory
 so now data transfer between host and device during runtime


 Kernel based on Rui's ofxOpenCL particle example opencl particles 001b.zip
 at http://code.google.com/p/ruisource/
 *****/


#include "PartyCLApp.h"

namespace entropy
{
    //--------------------------------------------------------------
    void PartyCLApp::setup()
    {
        ofBackground(ofColor::black);
        ofSetLogLevel(OF_LOG_VERBOSE);
        ofSetVerticalSync(false);

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
        params.add(timestep.set("timestep", 0.016, 0.01, 0.02));
        params.add(clusterScale.set("cluster scale", 1.54, 0.1, 10.0));
        params.add(velocityScale.set("velocity scale", 8.0, 0.0, 1000.0));
        params.add(softening.set("softening", 0.1, 0.0, 1.0));
        params.add(damping.set("damping", 1.0, 0.0, 1.0));
        params.add(pointSize.set("point size", 1.0f, 0.01f, 16.0f));
        ofAddListener(params.parameterChangedE(), this, &PartyCLApp::paramsChanged);

        // Set Gui.
        guiPanel.setup(params, "partycl.xml");
        guiPanel.loadFromFile("partycl.xml");

        bGuiVisible = true;

        // Load the first preset.
        presetIndex = 0;
        loadPreset();

        // Set the number of bodies and configure appropriately.
        numBodies = 1024 * 12;

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

        // Allocate host memory.
        hPos = new float[numBodies*4];
        hVel = new float[numBodies*4];
        hColor = new float[numBodies*4];

        // Init renderer.
        renderer = new ParticleRenderer();
        displayMode = ParticleRenderer::PARTICLE_SPRITES;

        // Start the show.
        bPaused = false;
        bReset = true;
    }

    //--------------------------------------------------------------
    void PartyCLApp::resetSimulation()
    {
        ofLogNotice("PartyCLApp::resetSim", "Resetting Nbody system...");

        randomizeBodies(activeConfig, hPos, hVel, hColor, clusterScale, velocityScale, numBodies);

        system->setArray(NBodySystem::ARRAY_POSITION, hPos);
        system->setArray(NBodySystem::ARRAY_VELOCITY, hVel);

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
