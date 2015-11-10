#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/MayaCamUI.h"
#include "cinder/Perlin.h"

#include "MarchingCubes.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class MarchingCubesApp : public App {
  public:
	void setup() override;
    void keyDown( KeyEvent event ) override;
	void draw() override;
    
    void polygonize();
    
    float           mTargetValue;
    int             mGridSize;
    gl::VboMeshRef  mMesh;
    MayaCamUI       mMayaCamUI;
};

void MarchingCubesApp::setup()
{
    CameraPersp cam;
    cam.setPerspective( 50.0f, getWindowAspectRatio(), 0.1f, 1000.0f );
    mMayaCamUI.setCurrentCam( cam );
    mMayaCamUI.connect( getWindow() );
    
    mTargetValue    = 50.0f;
    mGridSize       = 16;
    
    polygonize();
}


void MarchingCubesApp::polygonize()
{
    auto triMesh                = TriMesh::create( TriMesh::Format().positions().normals() );
    static auto densityFunction = []( const vec3 &position ){
        static Perlin perlin;
        return perlin.fBm( position ) * 150.0f;
    };
    
    vec3 cell;
    float stepSize = 5.0f / (float) mGridSize;
    for( cell.x = 0; cell.x < mGridSize; cell.x++ ){
        for( cell.y = 0; cell.y < mGridSize; cell.y++ ){
            for( cell.z = 0; cell.z < mGridSize; cell.z++ ){
                MarchingCubes::march( triMesh, cell * stepSize, stepSize, mTargetValue, densityFunction );
            }
        }
    }
    
    mMesh = gl::VboMesh::create( *triMesh.get() );
    cout << triMesh->getNumTriangles() << endl;
}

void MarchingCubesApp::keyDown( KeyEvent event )
{
    switch ( event.getCode() ) {
        case KeyEvent::KEY_UP:
            if( mTargetValue < 1000.0 ) mTargetValue *= 1.1;
            cout << mTargetValue << endl;
            break;
        case KeyEvent::KEY_DOWN:
            if( mTargetValue > 1.0 ) mTargetValue /= 1.1;
            cout << mTargetValue << endl;
            break;
        case KeyEvent::KEY_RIGHT:
            if( mGridSize < 256) mGridSize *= 2;
            cout << mGridSize << endl;
            break;
        case KeyEvent::KEY_LEFT:
            if( mGridSize > 16 ) mGridSize /= 2;
            cout << mGridSize << endl;
            break;
        default:
            break;
    }
    polygonize();
}

void MarchingCubesApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
    
    gl::setMatrices( mMayaCamUI.getCamera() );
    gl::enableWireframe();
    gl::enableAdditiveBlending();
    gl::color( ColorA::gray( 1.0f, 0.02f ) );
    if( mMesh )
        gl::draw( mMesh );
    
}

CINDER_APP( MarchingCubesApp, RendererGl )
