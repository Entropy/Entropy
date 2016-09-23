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
            "src/entropy/inflation/GPUMarchingCubes.cpp",
            "src/entropy/inflation/GPUMarchingCubes.h",
            "src/entropy/inflation/NoiseField.cpp",
            "src/entropy/inflation/NoiseField.h",
            "src/entropy/inflation/TransitionParticles.cpp",
            "src/entropy/inflation/TransitionParticles.h",
            "src/entropy/scene/Inflation.cpp",
            "src/entropy/scene/Inflation.h",
            "src/main.cpp",
            "src/ofApp.cpp",
            "src/ofApp.h",
            "../../Resources/data/entropy/render/PostEffects/shaders/brightnessThreshold.frag",
            "../../Resources/data/entropy/render/PostEffects/shaders/directionalBlur.frag",
            "../../Resources/data/entropy/render/PostEffects/shaders/directionalBlur.vert",
            "../../Resources/data/entropy/render/PostEffects/shaders/frag_tonemap.glsl",
            "../../Resources/data/entropy/render/PostEffects/shaders/fullscreenTriangle.vert",
            "../../Resources/data/entropy/render/PostEffects/shaders/passthrough_vert.glsl",
            "../../Resources/data/entropy/render/Renderers/shaders/wireframeFillRender.frag",
            "../../Resources/data/entropy/render/Renderers/shaders/wireframeFillRender.vert",
            "../../Resources/data/entropy/scene/Inflation/shaders/compute_clear_color.glsl",
            "../../Resources/data/entropy/scene/Inflation/shaders/compute_noise4d.glsl",
            "../../Resources/data/entropy/scene/Inflation/shaders/compute_particles.glsl",
            "../../Resources/data/entropy/scene/Inflation/shaders/frag_blur.glsl",
            "../../Resources/data/entropy/scene/Inflation/shaders/frag_bright.glsl",
            "../../Resources/data/entropy/scene/Inflation/shaders/frag_tonemap.glsl",
            "../../Resources/data/entropy/scene/Inflation/shaders/marching_cubes_geom.glsl",
            "../../Resources/data/entropy/scene/Inflation/shaders/passthrough_vert.glsl",
            "../../Resources/data/entropy/scene/Inflation/shaders/transition_particle.frag",
            "../../Resources/data/entropy/scene/Inflation/shaders/transition_particle.vert",
            "../../Resources/data/entropy/scene/Inflation/shaders/vert_blur.glsl",
            "../../Resources/data/entropy/scene/Inflation/shaders/vert_full_quad.glsl",
            "../../Resources/data/entropy/scene/Inflation/shaders/volumetrics_frag.glsl",
            "../../Resources/data/entropy/scene/Inflation/shaders/volumetrics_vertex.glsl",
            "addons.make",
        ]

        of.addons: [
            '../EntropyLib',
            '../../addons/ofxRange',
            '../../addons/ofxImGui',
            '../../addons/ofxPreset',
            '../../addons/ofxSet',
            '../../addons/ofxTextInputField',
            '../../addons/ofxTextureRecorder',
            '../../addons/ofxVolumetrics',
            '../../addons/ofxTimeline',
            '../../addons/ofxTimecode',
            '../../addons/ofxTween',
            '../../addons/ofxMSATimer',
            '../../addons/ofxWarp',
            '../../addons/ofxEasing',
            '../../addons/ofxObjLoader',
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

    //property bool usePoco: false;
    property bool makeOF: true;
    references: [FileInfo.joinPaths(of_root, "/libs/openFrameworksCompiled/project/qtcreator/openFrameworks.qbs")]
}
