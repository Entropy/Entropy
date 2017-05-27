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
            "../../Resources/data/entropy/scene/Bubbles/shaders/burst.frag",
            "../../Resources/data/entropy/scene/Bubbles/shaders/burst.vert",
            "../../Resources/data/entropy/scene/Bubbles/shaders/copy3D.frag",
            "../../Resources/data/entropy/scene/Bubbles/shaders/drop3D.frag",
            "../../Resources/data/entropy/scene/Bubbles/shaders/layer.geom",
            "../../Resources/data/entropy/scene/Bubbles/shaders/passthru.vert",
            "../../Resources/data/entropy/scene/Bubbles/shaders/reveal.frag",
            "../../Resources/data/entropy/scene/Bubbles/shaders/reveal.vert",
            "../../Resources/data/entropy/scene/Bubbles/shaders/ripple.frag",
            "../../Resources/data/entropy/scene/Bubbles/shaders/ripple3D.frag",
            "../../Resources/data/entropy/scene/Bubbles/shaders/volumetrics_frag.glsl",
            "../../Resources/data/entropy/scene/Bubbles/shaders/volumetrics_vertex.glsl",
            'src/entropy/bubbles/Bursts.cpp',
            'src/entropy/bubbles/Bursts.h',
            'src/entropy/bubbles/CmbSceneCL2D.cpp',
            'src/entropy/bubbles/CmbSceneCL2D.h',
            'src/entropy/bubbles/CmbSceneCL3D.cpp',
            'src/entropy/bubbles/CmbSceneCL3D.h',
            'src/entropy/bubbles/Constants.h',
            'src/entropy/bubbles/OpenCLImage3D.cpp',
            'src/entropy/bubbles/OpenCLImage3D.h',
            'src/entropy/bubbles/PoolBase.cpp',
            'src/entropy/bubbles/PoolBase.h',
            'src/entropy/bubbles/PoolGL2D.cpp',
            'src/entropy/bubbles/PoolGL2D.h',
            'src/entropy/bubbles/PoolGL3D.cpp',
            'src/entropy/bubbles/PoolGL3D.h',
            'src/entropy/bubbles/ofxFbo.cpp',
            'src/entropy/bubbles/ofxFbo.h',
            'src/entropy/scene/Bubbles.cpp',
            'src/entropy/scene/Bubbles.h',
            'src/main.cpp',
            'src/ofApp.cpp',
            'src/ofApp.h',
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

    property bool usePoco: false
    references: [FileInfo.joinPaths(of_root, "/libs/openFrameworksCompiled/project/qtcreator/openFrameworks.qbs")]
}
