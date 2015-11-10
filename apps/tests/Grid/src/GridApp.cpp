#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/MayaCamUI.h"
#include "cinder/Utilities.h"
#include "cinder/Log.h"

#include "CinderImGui.h"
#include "Grid.h"
#include "Watchdog.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class GridApp : public App {
  public:
	void setup() override;
    void draw() override;
    void keyDown( KeyEvent event ) override;
    void resize() override;
    
    MayaCamUI       mMayaCamUI;
    gl::BatchRef    mBatch;
    gl::TextureRef  mTexture;
    Grid            mGrid;
    vector<vec4> positions, initialPositions;
};

void GridApp::setup()
{
    
    ui::initialize();
    
    // get the absolute path to the data
    fs::path p = getAssetPath( "gadget_id_ip_fp_0p25t128.txt" );//getOpenFilePath();
    
    // quit if the file doesn't exists
    if( p.empty() || !fs::exists( p ) )
        quit();
    
    // open the file and parse it line by line
    ifstream file( p.c_str(), std::ios_base::in | std::ios_base::binary );
    string line;
    
    uint32_t index = 0;
    while( std::getline( file, line ) ){
        
        // split the line into strings
        auto splits = split( line, " " );
        
        // and extract the data
        int particleID  = stoi( splits[0] );
        vec4 initialPos = vec4( vec3( stof( splits[1] ), stof( splits[2] ), stof( splits[3] ) ), 0.0f );
        vec4 finalPos   = vec4( vec3( stof( splits[4] ), stof( splits[5] ), stof( splits[6] ) ), 0.0f );
        
        // store the positions
        initialPositions.push_back( initialPos );
        positions.push_back( finalPos );
    }
    
    // find data bounding box
    vec3 min = vec3( numeric_limits<float>::max() );
    vec3 max = vec3( numeric_limits<float>::min() );
    for( auto p : positions ){
        if( p.x > max.x ) max.x = p.x;
        if( p.y > max.y ) max.y = p.y;
        if( p.z > max.z ) max.z = p.z;
        if( p.x < min.x ) min.x = p.x;
        if( p.y < min.y ) min.y = p.y;
        if( p.z < min.z ) min.z = p.z;
    }
    
    mGrid = Grid( AxisAlignedBox3f( min, max ), 6 );
    
    cout << mGrid.getNumCells() << endl;
    
    int i = 0;
    for( auto &p : positions ){
        mGrid.addPoint( vec3( p ), i );
        i++;
    }
    /*vec3 size       = max - min;
    vec3 halfSize   = size * 0.5f;
    
    // center everything
    for( auto &p : positions ){
        p = vec4( vec3( p ) - halfSize, p.w );
    }
    for( auto &p : initialPositions ){
        p = vec4( vec3( p ) - halfSize, p.w );
    }*/
    
    // create a vbo mesh with the data
    geom::BufferLayout layout0;
    layout0.append( geom::POSITION, 4, 0, 0 );
    
    geom::BufferLayout layout1;
    layout1.append( geom::CUSTOM_0, 4, 0, 0 );
    
    auto vbo0       = gl::Vbo::create( GL_ARRAY_BUFFER, positions.size() * sizeof(vec4), positions.data(), GL_STATIC_DRAW );
    auto vbo1       = gl::Vbo::create( GL_ARRAY_BUFFER, initialPositions.size() * sizeof(vec4), initialPositions.data(), GL_STATIC_DRAW );
    auto vboMesh    = gl::VboMesh::create( positions.size(), GL_POINTS, { { layout0, vbo0 }, { layout1, vbo1 } } );
    
    mBatch              = gl::Batch::create( vboMesh, gl::getStockShader( gl::ShaderDef().color() ) );
    
    // load and watch textures
    wd::watch( "textures/particle.png", [this]( const fs::path &path ) {
        mTexture = gl::Texture::create( loadImage( loadAsset( "textures/particle.png" ) ), gl::Texture::Format().mipmap().minFilter( GL_LINEAR_MIPMAP_LINEAR ).magFilter( GL_LINEAR ).immutableStorage() );
    } );
    
    
    // load, compile and watch shaders
    wd::watch( "shaders/pointSprite.*", [this]( const fs::path &path ) {
        try {
            auto shader = gl::GlslProg::create( gl::GlslProg::Format()
                                                   .vertex( loadAsset( "shaders/pointSprite.vert" ) )
                                                   .fragment( loadAsset( "shaders/pointSprite.frag" ) )
                                                   .attrib( geom::CUSTOM_0, "aInitialPosition" ) );
            
            mBatch->replaceGlslProg( shader );
        }
        catch( gl::GlslProgExc exc ) { CI_LOG_E( exc.what() ); }
    } );
    
    // setup camera
    CameraPersp cam;
    cam.setPerspective( 50.0f, getWindowAspectRatio(), 0.1f, 1000.0f );
    mMayaCamUI.setCurrentCam( cam );
    mMayaCamUI.connect( getWindow() );
}


void GridApp::resize()
{
    CameraPersp cam = mMayaCamUI.getCamera();
    cam.setPerspective( 50.0f, getWindow()->getAspectRatio(), 0.1f, 1000.0f );
    mMayaCamUI.setCurrentCam( cam );
}

void GridApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
        gl::viewport( ivec2(0), getWindowSize() );
    gl::setMatrices( mMayaCamUI.getCamera() );
    
    
    gl::enableAdditiveBlending();
    if( mTexture && mBatch ){
        gl::ScopedState programPointSize( GL_PROGRAM_POINT_SIZE, true );
        gl::ScopedTextureBind tex( mTexture );
        mBatch->getGlslProg()->uniform( "uPointScale", 0.05f * (float) getWindowHeight() / math<float>::tan( mMayaCamUI.getCamera().getFov() * 0.5f * M_PI / 180.0f ) );
        mBatch->getGlslProg()->uniform( "uFog", 0.0001f );
        mBatch->draw();
    }
    
    
    static auto stockShader = gl::getStockShader( gl::ShaderDef().color() );
    gl::ScopedGlslProg shading( stockShader );
    gl::drawSphere( vec3( getMousePos(), 0 ), 100, 16 );
    Grid::Cell cell = mGrid.getRegion( vec3( getMousePos(), 0 ), 100 );
    cout << cell.size() << endl;
    for( auto id : cell ){
        gl::drawSphere( vec3( positions[id] ), 10, 16 );
    }
}

void GridApp::keyDown( KeyEvent event )
{
    switch ( event.getCode() ) {
        case KeyEvent::KEY_f:
            setFullScreen( !isFullScreen() );
            break;
            
        default:
            break;
    }
}
CINDER_APP( GridApp, RendererGl )
