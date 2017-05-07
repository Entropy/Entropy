import qbs
import qbs.Process
import qbs.File
import qbs.FileInfo
import qbs.TextFile
import "../../../libs/openFrameworksCompiled/project/qtcreator/ofApp.qbs" as ofApp

Project{
    property string of_root: '../../..'

    ofApp {
        name: { return FileInfo.baseName(path) }

        files: [
            "../../Resources/data/entropy/render/PostEffects/shaders/brightnessThreshold.frag",
            "../../Resources/data/entropy/render/PostEffects/shaders/directionalBlur.frag",
            "../../Resources/data/entropy/render/PostEffects/shaders/directionalBlur.vert",
            "../../Resources/data/entropy/render/PostEffects/shaders/frag_tonemap.glsl",
            "../../Resources/data/entropy/render/PostEffects/shaders/fullscreenTriangle.vert",
            "../../Resources/data/entropy/render/PostEffects/shaders/inc/clusteredShading.glsl",
            "../../Resources/data/entropy/render/PostEffects/shaders/inc/computeBrdfLut.frag",
            "../../Resources/data/entropy/render/PostEffects/shaders/inc/math.glsl",
            "../../Resources/data/entropy/render/PostEffects/shaders/inc/ofDefaultUniforms.glsl",
            "../../Resources/data/entropy/render/PostEffects/shaders/inc/ofDefaultVertexInAttributes.glsl",
            "../../Resources/data/entropy/render/PostEffects/shaders/inc/pbr.glsl",
            "../../Resources/data/entropy/render/PostEffects/shaders/inc/viewData.glsl",
            "../../Resources/data/entropy/render/PostEffects/shaders/passthrough_vert.glsl",
            "../../Resources/data/entropy/render/PostEffects/shaders/vert_full_quad.glsl",
            'src/main.cpp',
            'src/ofApp.cpp',
            'src/ofApp.h',
        ]

        of.addons: [
            '../../Projects/EntropyRender',
            '../../Projects/EntropyUtil',
            'ofxGui',
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
        of.frameworks: []       // osx only, additional frameworks to link with the project

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

    property bool usePoco: true // enables / disables compiling poco with OF
    // (will disable some functionality)

    property bool makeOF: true  // use makfiles to compile the OF library
    // will compile OF only once for all your projects
    // otherwise compiled per project with qbs

    references: [FileInfo.joinPaths(of_root, "/libs/openFrameworksCompiled/project/qtcreator/openFrameworks.qbs")]
}
