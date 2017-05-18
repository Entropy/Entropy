import qbs
import qbs.Process
import qbs.File
import qbs.FileInfo
import qbs.TextFile
import "../../../libs/openFrameworksCompiled/project/qtcreator/ofApp.qbs" as ofApp

Project{
    property string of_root: "../../.."

    ofApp {
        name: { return FileInfo.baseName(path) }

        files: [
            "src/entropy/particles/Environment.cpp",
            "src/entropy/particles/Environment.h",
            "src/entropy/particles/Octree.cpp",
            "src/entropy/particles/Octree.h",
            "src/entropy/particles/Octree.inl",
            "src/entropy/particles/Particle.cpp",
            "src/entropy/particles/Particle.h",
            "src/entropy/particles/ParticleSystem.cpp",
            "src/entropy/particles/ParticleSystem.h",
            "src/entropy/particles/Photons.cpp",
            "src/entropy/particles/Photons.h",
//            "src/entropy/scene/Particles.cpp",
//            "src/entropy/scene/Particles.h",
            "src/main.cpp",
            "src/ofApp.cpp",
            "src/ofApp.h",
        ]

        Group{
            name: "shaders"
            files: [
                "../../Resources/data/entropy/render/PostEffects/shaders/brightnessThreshold.frag",
                "../../Resources/data/entropy/render/PostEffects/shaders/directionalBlur.frag",
                "../../Resources/data/entropy/render/PostEffects/shaders/directionalBlur.vert",
                "../../Resources/data/entropy/render/PostEffects/shaders/frag_tonemap.glsl",
                "../../Resources/data/entropy/render/PostEffects/shaders/fullscreenTriangle.vert",
                "../../Resources/data/entropy/render/PostEffects/shaders/passthrough_vert.glsl",
                "../../Resources/data/entropy/render/Renderers/shaders/wireframeFillRender.frag",
                "../../Resources/data/entropy/render/Renderers/shaders/wireframeFillRender.vert",
                "../../Resources/data/entropy/scene/Particles/shaders/inc/clusteredShading.glsl",
                "../../Resources/data/entropy/scene/Particles/shaders/inc/computeBrdfLut.frag",
                "../../Resources/data/entropy/scene/Particles/shaders/inc/math.glsl",
                "../../Resources/data/entropy/scene/Particles/shaders/inc/ofDefaultUniforms.glsl",
                "../../Resources/data/entropy/scene/Particles/shaders/inc/ofDefaultVertexInAttributes.glsl",
                "../../Resources/data/entropy/scene/Particles/shaders/inc/pbr.glsl",
                "../../Resources/data/entropy/scene/Particles/shaders/inc/toneMapping.glsl",
                "../../Resources/data/entropy/scene/Particles/shaders/inc/viewData.glsl",
                "../../Resources/data/entropy/scene/Particles/shaders/particle.frag",
                "../../Resources/data/entropy/scene/Particles/shaders/particle.vert",
                "../../Resources/data/entropy/scene/Particles/shaders/photon_draw.frag",
                "../../Resources/data/entropy/scene/Particles/shaders/photon_draw.vert",
                "../../Resources/data/entropy/scene/Particles/shaders/photon_update.frag",
                "../../Resources/data/entropy/scene/Particles/shaders/photon_update.vert",
                "../../Resources/data/entropy/scene/Particles/shaders/skybox.frag",
                "../../Resources/data/entropy/scene/Particles/shaders/skybox.vert",
                "../../Resources/data/entropy/scene/Particles/shaders/wall.frag",
                "../../Resources/data/entropy/scene/Particles/shaders/wall.vert",
            ]
        }

        of.addons: [
            '../EntropyRender',
            '../EntropyUtil',
            '../../addons/ofxEasing',
            '../../addons/ofxRange',
            '../../addons/ofxSerialize',
            '../../addons/ofxTbb',
            '../../addons/ofxSet',
            '../../addons/ofxTextInputField',
            '../../addons/ofxTextureRecorder',
            '../../addons/ofxVideoRecorder',
            '../../addons/ofxTimeline',
            '../../addons/ofxTimecode',
            '../../addons/ofxGpuParticles',
            '../../addons/ofxObjLoader',
            'ofxGui',
            'ofxXmlSettings',
        ]

        // additional flags for the project. the of module sets some
        // flags by default to add the core libraries, search paths...
        // this flags can be augmented through the following properties:
        of.pkgConfigs: []       // list of additional system pkgs to include
        of.includePaths: []     // include search paths
        of.cFlags: []           // flags passed to the c compiler
        of.cxxFlags: []         // flags passed to the c++ compiler
        of.linkerFlags: []      // flags passed to the linker
        of.defines: []          // defines are passed as -D to the compiler
                                // and can be checked with #ifdef or #if in the code

        // other flags can be set through the cpp module: http://doc.qt.io/qbs/cpp-module.html
        // eg: this will enable ccache when compiling
        //
        // cpp.compilerWrapper: 'ccache'

        Depends{
            name: "cpp"
        }

        // common rules that parse the include search paths, core libraries...
        Depends{
            name: "of"
        }

        // dependency with the OF library
        Depends{
            name: "openFrameworks"
        }
    }

    references: [FileInfo.joinPaths(of_root, "/libs/openFrameworksCompiled/project/qtcreator/openFrameworks.qbs")]
}
