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
            'src/entropy/surveys/DataSet.cpp',
            'src/entropy/surveys/DataSet.h',
            'src/entropy/surveys/GaussianMapTexture.cpp',
            'src/entropy/surveys/GaussianMapTexture.h',
            'src/entropy/surveys/TravelCamPath.cpp',
            'src/entropy/surveys/TravelCamPath.h',
            'src/main.cpp',
            'src/ofApp.cpp',
            'src/ofApp.h',
        ]

        of.addons: [
            'ofxGui',
            'ofxXmlSettings',
            '../../addons/ofxEasing',
            '../../addons/ofxHDF5',
            '../../addons/ofxObjLoader',
            '../../addons/ofxRange',
            '../../addons/ofxSet',
            '../../addons/ofxTextInputField',
            '../../addons/ofxTextureRecorder',
            '../../addons/ofxTimecode',
            '../../addons/ofxTimeline',
            '../../addons/ofxSerialize',
            '../EntropyGeom',
            '../EntropyRender',
            '../EntropyUtil',
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
