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
            "bin/data/marching-cubes.json",
            "bin/data/noise-field.json",
            "bin/data/render.json",
            "bin/data/shaders/compute_noise4d.glsl",
            "bin/data/shaders/frag_blur.glsl",
            "bin/data/shaders/frag_bright.glsl",
            "bin/data/shaders/frag_tonemap.glsl",
            "bin/data/shaders/marching_cubes_geom.glsl",
            "bin/data/shaders/normalShader.frag",
            "bin/data/shaders/normalShader.vert",
            "bin/data/shaders/passthrough_vert.glsl",
            "bin/data/shaders/vert_full_quad.glsl",
            "bin/data/shaders/volumetrics_frag.glsl",
            "bin/data/shaders/volumetrics_vertex.glsl",
            "src/main.cpp",
            "src/Constants.h",
            "src/GPUMarchingCubes.cpp",
            "src/GPUMarchingCubes.h",
            "src/InflationApp.cpp",
            "src/InflationApp.h",
            "src/NoiseField.cpp",
            "src/NoiseField.h",
        ]

        of.addons: [
            'ofxGui',
            '../../addons/ofxVolumetrics',
            '../../addons/ofxTextureRecorder'
        ]

        // additional flags for the project. the of module sets some
        // flags by default to add the core libraries, search paths...
        // this flags can be augmented through the following properties:
        of.pkgConfigs: ['tbb']       // list of additional system pkgs to include
        of.includePaths: []     // include search paths
        of.cFlags: ['-masm=intel','-fno-math-errno']           // flags passed to the c compiler
        of.cxxFlags: ['-masm=intel','-fno-math-errno']         // flags passed to the c++ compiler
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
