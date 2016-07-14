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
            "bin/data/shaders/int2float.glsl",
            "bin/data/shaders/maprange.glsl",
            "bin/data/shaders/particles2texture3d.glsl",
            "bin/data/shaders/render.frag",
            "bin/data/shaders/render.vert",
            "bin/data/shaders/volumetrics_frag.glsl",
            "bin/data/shaders/volumetrics_vertex.glsl",
            "bin/data/shaders/voxels2texture3d.glsl",
            "src/Constants.h",
            "src/Particle.h",
            "src/SequenceRamses.cpp",
            "src/SequenceRamses.h",
            "src/SnapshotRamses.cpp",
            "src/SnapshotRamses.h",
            "src/Vapor3DTexture.cpp",
            "src/Vapor3DTexture.h",
            "src/VaporOctree.cpp",
            "src/VaporOctree.h",
            "src/main.cpp",
            "src/ofApp.cpp",
            "src/ofApp.h",
        ]

        of.addons: [
            '../../addons/ofxHDF5',
            '../../addons/ofxImGui',
            '../../addons/ofxRange',
            '../../addons/ofxSet',
            '../../addons/ofxTimeline',
            '../../addons/ofxMSATimer',
            '../../addons/ofxTextInputField',
            '../../addons/ofxTween',
            '../../addons/ofxTimecode',
            '../../addons/ofxVolumetrics',
            '../../addons/ofxLibfbi',
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
