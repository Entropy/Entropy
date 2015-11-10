#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/MayaCamUI.h"
#include "cinder/Utilities.h"
#include "cinder/Log.h"
#include "cinder/Timeline.h"
#include "cinder/Timer.h"
#include "cinder/qtime/AvfWriter.h"
#include "glm/gtc/packing.hpp"

#include <cereal/archives/binary.hpp>

#include "FontAwesome.h"
#include "CinderImGui.h"
#include "Grid.h"
#include "Watchdog.h"
#include "MarchingCubes.h"
#include "Particle.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class CosmicWebApp : public App {
public:
    void setup() override;
    void draw() override;
    void resize() override;
    void cleanup() override { mMovieExporter.reset(); }
    void mouseMove( MouseEvent event ) override { mMousePos = event.getPos(); }
    void mouseDown( MouseEvent event ) override;
    void keyDown( KeyEvent event ) override;
    
    int pickParticles( const ivec2 &mouse );
    
    
    MayaCamUI               mMayaCamUI;
    CameraPersp             mCamera, mCameraStart, mCameraEnd;
    bool                    mMainCamera = false;
    
    ParticleSet             mDarkMatter;
    gl::GlslProgRef         mDarkMatterShader, mDarkMatterIdShader;
    float                   mTime = 0.0f;
    qtime::MovieWriterRef   mMovieExporter;
    TimelineRef             mMovieTimeline;
    gl::FboRef              mFbo;
    vec3                    mCameraEye, mCameraTarget;
    bool                    mZoomed;
    float                   pointScale = 1.4f;
    float                   alphaScale = 1.2f;
    double                  mLastClickTime = 0.0;
    bool                    mShowUI = false;
    
    ivec2 mMousePos;
};


namespace ImGui {
    
bool IconButton( const std::string &icon )
{
    ui::ScopedFont fontAwesome( "fontawesome" );
    return ui::TextButton( icon.c_str() );
}
    
}

//#define ENCODE
void CosmicWebApp::setup()
{
    
    // setup ui
    ui::initialize( ui::Options()
                   .fonts( { { getAssetPath( "fonts/boblight.ttf" ), 11 }, { getAssetPath( "fonts/bobbold.ttf" ), 16 }, { getAssetPath( "fonts/fontawesome.ttf" ), 11 } } )
                   .fontGlyphRanges( "fontawesome", { fa::charsRange().first, fa::charsRange().second } ) );
    
    
    // get the absolute path to the data
    fs::path dataDirectory  = getAssetPath( "data" );// ( getAppPath().parent_path().parent_path().parent_path().parent_path().parent_path().parent_path() / "common/data" );
    fs::path p              = dataDirectory / "DarkMatter.cr2";// getOpenFilePath( dataDirectory );
    
    // quit if the file doesn't exists
    if( p.empty() || !fs::exists( p ) )
        quit();
    
#ifdef ENCODE
    // open the file and rewrite it to our format
    mDarkMatter.readGadgetAscii( p );
    mDarkMatter.write( getSaveFilePath() );
    quit();
#endif
    
    // read the set from file
    //mDarkMatter.resample( { dataDirectory / "DarkMatter2.cr2" }, 1000 );
    mDarkMatter.read( dataDirectory / "DarkMatter.cr2", dataDirectory / "DarkMatter.crd" );
    
    
    // load, compile and watch shaders
    wd::watch( "shaders/pointSprite.*", [this]( const fs::path &path ) {
        try {
            mDarkMatterShader = gl::GlslProg::create( gl::GlslProg::Format()
                                                     .vertex( loadAsset( "shaders/pointSprite.vert" ) )
                                                     .fragment( loadAsset( "shaders/pointSprite.frag" ) )
                                                     .attrib( geom::CUSTOM_0, "aVelocity" )
                                                     .attrib( geom::CUSTOM_1, "aId" ) );
        }
        catch( gl::GlslProgExc exc ) { CI_LOG_E( exc.what() ); }
    } );
    
    // load, compile and watch shaders
    wd::watch( "shaders/pointSpriteId.*", [this]( const fs::path &path ) {
        try {
            mDarkMatterIdShader = gl::GlslProg::create( gl::GlslProg::Format()
                                                     .vertex( loadAsset( "shaders/pointSprite.vert" ) )
                                                     .fragment( loadAsset( "shaders/pointSpriteId.frag" ) )
                                                     .attrib( geom::CUSTOM_0, "aVelocity" )
                                                     .attrib( geom::CUSTOM_1, "aId" ) );
        }
        catch( gl::GlslProgExc exc ) { CI_LOG_E( exc.what() ); }
    } );
    
    vec2 res( 1920 / 2, 1080 / 2 );
    gl::Fbo::Format format;
    format.attachment( GL_COLOR_ATTACHMENT0, gl::Texture2d::create( res.x, res.y, gl::Texture2d::Format().internalFormat( GL_RGBA ) ) );
    format.attachment( GL_COLOR_ATTACHMENT1, gl::Texture2d::create( res.x, res.y, gl::Texture2d::Format().internalFormat( GL_RGBA ) ) );
    
    mFbo = gl::Fbo::create( res.x, res.y, format );
    
    // setup camera
    mCamera.setPerspective( 50.0f, getWindowAspectRatio(), 0.1f, 50000.0f );
    mCamera.setEyePoint( mDarkMatter.getBounds().getCenter() + vec3(700,0,0) );
    mCamera.setCenterOfInterestPoint( mDarkMatter.getBounds().getCenter() );
    mCameraStart = mCameraEnd = mCamera;
    mMayaCamUI = MayaCamUI( &mCamera );
    mMayaCamUI.connect( getWindow(), 1 );
}


void CosmicWebApp::resize()
{
    vec2 res = getWindowSize();
    gl::Fbo::Format format;
    format.attachment( GL_COLOR_ATTACHMENT0, gl::Texture2d::create( res.x, res.y, gl::Texture2d::Format().internalFormat( GL_RGBA ) ) );
    format.attachment( GL_COLOR_ATTACHMENT1, gl::Texture2d::create( res.x, res.y, gl::Texture2d::Format().internalFormat( GL_RGBA ) ) );
    
    mFbo = gl::Fbo::create( res.x, res.y, format );
    //CameraPersp cam = mMayaCamUI.getCamera();
    //cam.setPerspective( 50.0f, getWindow()->getAspectRatio(), 0.1f, 10000.0f );
    //mMayaCamUI.setCurrentCam( cam );
    
    mCamera.setAspectRatio( getWindow()->getAspectRatio() );
    mCameraStart.setAspectRatio( getWindow()->getAspectRatio() );
    mCameraEnd.setAspectRatio( getWindow()->getAspectRatio() );
}
uint32_t colorToIndex( const ci::ColorA8u &color )
{
    return ((color.a << 24) | (color.r << 16) | (color.g << 8) | (color.b));
}

void CosmicWebApp::draw()
{
    
    static int blending = 2;
    
    static bool depthTesting = false;
    static bool debug = false;
    static vec3 low(0.16,0.09,0.2), high(0.34,0.11,0.11);
    const char* blendingTypes[3] = { "Disabled", "Alpha", "Additive" };
    static float velocityScale = 0.035;
    static float densitySmoothing = 0.035f;
    static bool drawIds = false;
    
    if( mShowUI ){
        ui::Begin( "Entropy" );
        
        ui::InputFloat( "Velocity Scale", &velocityScale, 0.001f );
        ui::InputFloat( "Density Smoothing", &densitySmoothing, 0.01f );
        ui::InputFloat( "Point Scale", &pointScale, 0.1f );
        ui::InputFloat( "Alpha Scale", &alphaScale, 0.1f );
        ui::SliderFloat( "###Time", &mTime, 0.0f, 1.0f );
        ui::SameLine();
        {
            ui::ScopedFont font( "fontawesome" );
            if( ui::IconButton( fa::clock_o() ) ){
                if( mTime <= 0.001f ) timeline().appendToPtr( &mTime, 1.0f, 10.0f, EaseInOutQuad() );
                else timeline().appendToPtr( &mTime, 0.0f, 10.0f, EaseInOutQuad() );
            }
        }
        if( ui::CollapsingHeader( "Rendering" ) ){
            static float lgray = 1.0f;
            static float hgray = 1.0f;
            ui::ColorEdit3( "LowGradient", &low[0] );
            if( ui::SliderFloat( "###lGray", &lgray, 0, 1.0f ) ) low = vec3(lgray);
            ui::SameLine(); if( ui::SmallButton( "reset###1" ) ) low = vec3(0.16,0.09,0.2);
            ui::ColorEdit3( "HighGradient", &high[0] );
            if( ui::SliderFloat( "###hGray", &hgray, 0, 1.0f ) ) high = vec3(hgray);
            ui::SameLine(); if( ui::SmallButton( "reset###2" ) ) high = vec3(0.34,0.11,0.11);
            ui::Combo( "Blending", &blending, blendingTypes, 3 );
            ui::Checkbox( "DepthTesting", &depthTesting );
            ui::Checkbox( "Debug Mesh", &debug );
            ui::Checkbox( "Draw Ids Texture", &drawIds );
        }
        if( ui::CollapsingHeader( "Export" ) ){
            ui::Button( "Export Quicktime" );
        }
        
        ui::End();
    
    }
    
    switch ( blending ) {
        case 0: gl::disableAlphaBlending(); break;
        case 1: gl::enableAlphaBlending(); break;
        case 2: gl::enableAdditiveBlending(); break;
    }
    
    if( mDarkMatterShader && mDarkMatter.getVboMesh() ){
        
        gl::ScopedFramebuffer fbo( mFbo );
        
        gl::drawBuffer( GL_COLOR_ATTACHMENT0 );
        
        gl::ScopedState programPointSize( GL_PROGRAM_POINT_SIZE, true );
        gl::ScopedGlslProg shading( mDarkMatterShader );
        gl::viewport( ivec2(0), mFbo->getSize() );
        gl::clear( Color( 0, 0, 0 ) );
        
        if( mMainCamera ){
        //    mCamera.setEyePoint( glm::mix( mCameraStart.getEyePoint(), mCameraEnd.getEyePoint(), mTime ) );
        //    mCamera.setOrientation( glm::mix( mCameraStart.getOrientation(), mCameraEnd.getOrientation(), mTime ) );
        }
        
        auto cam = mCamera;// mMainCamera ? mCamera : mMayaCamUI.getCamera();
        gl::setMatrices( cam );
        
        mDarkMatterShader->uniform( "uPointScale", 0.05f * pointScale * (float) mFbo->getHeight() / math<float>::tan( cam.getFov() * 0.5f * M_PI / 180.0f ) );
        mDarkMatterShader->uniform( "uAlphaScale", alphaScale );
        mDarkMatterShader->uniform( "uFog", 0.0001f );
        
        mDarkMatterShader->uniform( "uTime", mTime );
        mDarkMatterShader->uniform( "uLowGradient", low );
        mDarkMatterShader->uniform( "uHighGradient", high );
        mDarkMatterShader->uniform( "uVelocityScale", velocityScale );
        mDarkMatterShader->uniform( "uDensitySmoothing", densitySmoothing );
        
        if( depthTesting ){
            gl::enableDepthRead();
            gl::enableDepthWrite();
        }
        else {
            gl::disableDepthRead();
            gl::disableDepthWrite();
        }
        
       // for( int y = -3; y < 3; y++ ){
         //   for( int z = -3; z < 3; z++ ){
              //  gl::ScopedModelMatrix model;
              //  gl::translate( vec3( 0, y, z ) * vec3( 0, 128, 128 ) * 0.1f );
        gl::draw( mDarkMatter.getVboMesh() );//, 0, count );
           // }
        //}
        
        //mBatch->draw();
        //mBatch->drawInstanced( 4 );
        
        static double lastFrameSec = getElapsedSeconds();
        if( mMovieExporter && mMovieTimeline ){
            mMovieExporter->addFrame( mFbo->getColorTexture()->createSource() );
            mMovieTimeline->stepTo( (float) mMovieExporter->getNumFrames() / 30.0f );
        }
        /*else if( mMovieTimeline ){
            mMovieTimeline->step( getElapsedSeconds() - lastFrameSec );
        }*/
        lastFrameSec = getElapsedSeconds();
        
        if( debug ){
            static auto stockColor = gl::getStockShader( gl::ShaderDef().color() );
            gl::ScopedGlslProg shader( stockColor );
            gl::ScopedColor color( ColorA::gray( 1.0f, 0.1f ) );
            gl::enableWireframe();
            gl::drawCube( mDarkMatter.getBounds().getCenter(), mDarkMatter.getBounds().getSize() );
            gl::drawCube( mDarkMatter.getBounds().getCenter(), vec3(1)      * vec3(0.1,1,1) );
            gl::drawCube( mDarkMatter.getBounds().getCenter(), vec3(10)     * vec3(0.1,1,1) );
            gl::drawCube( mDarkMatter.getBounds().getCenter(), vec3(100)    * vec3(0.1,1,1) );
            gl::drawCube( mDarkMatter.getBounds().getCenter(), vec3(1000)   * vec3(0.1,1,1) );
            gl::disableWireframe();
        }
        
        gl::ScopedGlslProg shadingId( mDarkMatterIdShader );
        mDarkMatterIdShader->uniform( "uPointScale", 0.02f * (float) mFbo->getHeight() / math<float>::tan( cam.getFov() * 0.5f * M_PI / 180.0f ) );
        mDarkMatterIdShader->uniform( "uTime", mTime );
        mDarkMatterIdShader->uniform( "uVelocityScale", velocityScale );
        mDarkMatterIdShader->uniform( "uDensitySmoothing", densitySmoothing );
        
        gl::enableDepthRead();
        gl::enableDepthWrite();
        gl::disableAlphaBlending();
        gl::drawBuffer( GL_COLOR_ATTACHMENT1 );
        gl::clear();
        gl::draw( mDarkMatter.getVboMesh() );
        gl::drawBuffer( GL_COLOR_ATTACHMENT0 );
        /*
        int selectedIndex = pickParticles( mMousePos );
        
        if( selectedIndex >= 0 && selectedIndex < mDarkMatter.getParticles().size() ){
            static auto stockColor = gl::getStockShader( gl::ShaderDef().color() );
            gl::ScopedGlslProg shader( stockColor );
            gl::enableWireframe();
            gl::drawCube( mDarkMatter.getParticles()[selectedIndex].getPosition(), vec3(10) );
            gl::disableWireframe();
        }*/
    }
    
    
    gl::disableDepthRead();
    gl::disableDepthWrite();
    
    gl::clear( Color( 0, 0, 0 ) );
    gl::setMatricesWindow( getWindowSize() );
    gl::viewport( ivec2(0), getWindowSize() );
    gl::ScopedColor color( ColorA::gray( 1.0f ) );
    if( !drawIds )
        gl::draw( mFbo->getColorTexture(), getWindowBounds() );
    else
        gl::draw( static_pointer_cast<gl::Texture2d>( mFbo->getTexture( GL_COLOR_ATTACHMENT1 ) ), getWindowBounds() );
    
    gl::enableAlphaBlending();
    gl::drawString( to_string( (int) getAverageFps() ), vec2(35) );
}

void CosmicWebApp::mouseDown( MouseEvent event )
{
    if( getElapsedSeconds() - mLastClickTime < 3.0 ){
        
        
        float duration = 16.0f;
        
        float zoomedScale = 0.4f;
        float zoomedAlpha = 2.2f;
        if( !mMovieTimeline )
           mMovieTimeline = Timeline::create();
        
        if( !mZoomed ){
            auto easing = EaseInOutQuad();
            mCameraEye      = mCamera.getEyePoint();
            mCameraTarget   = mCamera.getCenterOfInterestPoint();
            
            vec3 destinationEye     = mDarkMatter.getBounds().getCenter() + vec3(700,0,0);
            vec3 destinationTarget  = mDarkMatter.getBounds().getCenter();
            
            mMovieTimeline->applyPtr( &mCameraEye, destinationEye, duration, easing ).updateFn( [this](){
                mCamera.setEyePoint( mCameraEye );
            } ).pingPong().infinite();
            mMovieTimeline->applyPtr( &mCameraTarget, destinationTarget, duration, easing ).updateFn( [this](){
                mCamera.setCenterOfInterestPoint( mCameraTarget );
            } ).pingPong().infinite();
            mMovieTimeline->applyPtr( &pointScale, 1.4f, duration, easing ).pingPong().infinite();
            mMovieTimeline->applyPtr( &alphaScale, 1.2f, duration, easing ).pingPong().infinite();
            
            mZoomed = true;
        }
        else {
            auto easing = EaseInOutQuad();
            int selectedIndex = pickParticles( event.getPos() );
            
            if( selectedIndex >= 0 && selectedIndex < mDarkMatter.getParticles().size() ){
                
                vec3 pos        = mDarkMatter.getParticles()[selectedIndex].getPosition();
                mCameraEye      = mCamera.getEyePoint();
                mCameraTarget   = mCamera.getCenterOfInterestPoint();
                
                vec3 destinationEye     = pos + vec3( 80, -120, 0 );
                vec3 destinationTarget  = pos;
                
                mMovieTimeline->applyPtr( &mCameraEye, destinationEye, duration, easing ).updateFn( [this](){
                    mCamera.setEyePoint( mCameraEye );
                } ).pingPong().infinite();
                mMovieTimeline->applyPtr( &mCameraTarget, pos, duration, easing ).updateFn( [this](){
                    mCamera.setCenterOfInterestPoint( mCameraTarget );
                } ).pingPong().infinite();
                mMovieTimeline->applyPtr( &pointScale, zoomedScale, duration, easing ).pingPong().infinite();
                mMovieTimeline->applyPtr( &alphaScale, zoomedAlpha, duration, easing ).pingPong().infinite();
                
                mZoomed = false;
            }
        }
    }
    mLastClickTime = getElapsedSeconds();
}

int CosmicWebApp::pickParticles( const ivec2 &mouse )
{
    ivec2 sampleSize  = vec2(8);
    Surface8u pixels = mFbo->readPixels8u( Area( mMousePos - sampleSize / 2, mMousePos + sampleSize / 2 ), GL_COLOR_ATTACHMENT1 );
    
    std::map<int, int> voteCount;
    for( int32_t y = 0; y < pixels.getHeight(); ++y ) {
        for( int32_t x = 0; x < pixels.getWidth(); ++x ) {
            int colorNumber = colorToIndex( pixels.getPixel( ivec2( x, y ) ) );
            if( colorNumber != 0 )
                voteCount[colorNumber]++;
        }
    }
    
    int selectedIndex = -1;
    if( ! voteCount.empty() )
        selectedIndex = std::max_element( voteCount.begin(), voteCount.end(), voteCount.value_comp() )->first;
    
    return selectedIndex;
}

void CosmicWebApp::keyDown( KeyEvent event )
{
    switch ( event.getCode() ) {
        case KeyEvent::KEY_1:
            mMainCamera = !mMainCamera;
            //mMayaCamUI = MayaCamUI( &mCamera );
            break;
        case KeyEvent::KEY_2:
            mMayaCamUI = MayaCamUI( &mCameraStart );
            break;
        case KeyEvent::KEY_3:
            mMayaCamUI = MayaCamUI( &mCameraEnd );
            break;
        case KeyEvent::KEY_f:
            setFullScreen( !isFullScreen() );
            break;
        case KeyEvent::KEY_u:
            mShowUI = !mShowUI;
            break;
        case KeyEvent::KEY_o:
            // open the file and rewrite it to our format
            mDarkMatter.readGadgetAscii( getOpenFilePath() );
            mDarkMatter.write( getSaveFilePath() );
            break;
        case KeyEvent::KEY_SPACE:
            if( mTime <= 0.001f ){
                mMovieTimeline->appendToPtr( &mTime, 1.0f, 38.0f, EaseInOutQuad() );
            }
            else {
                mMovieTimeline->appendToPtr( &mTime, 0.0f, 38.0f, EaseInOutQuad() );
            }
            break;
        case KeyEvent::KEY_m:
        {
            if( !mMovieExporter ){
                fs::path path = getSaveFilePath();
                if( ! path.empty() ) {
                    auto format = qtime::MovieWriter::Format()
                    .codec( qtime::MovieWriter::H264 )
                    .fileType( qtime::MovieWriter::QUICK_TIME_MOVIE )
                    .jpegQuality( 1.0f )
                    .defaultFrameDuration( 1.0f / 30.0f );
                    mMovieExporter = qtime::MovieWriter::create( path, mFbo->getWidth(), mFbo->getHeight(), format );
                }
                /*
                mMovieTimeline = Timeline::create();
                
                if( mTime <= 0.001f ){
                    mMovieTimeline->applyPtr( &mTime, 1.0f, 30.0f, EaseInOutQuad() ).finishFn( [this](){
                        mMovieExporter->finish();
                        mMovieExporter.reset();
                        quit();
                    } );
                }
                else {
                    mMovieTimeline->applyPtr( &mTime, 0.0f, 30.0f, EaseInOutQuad() ).finishFn( [this](){
                        mMovieExporter->finish();
                        mMovieExporter.reset();
                        quit();
                    } );
                }*/
            }
            else {
                mMovieExporter->finish();
                mMovieExporter.reset();
            }
            
        } break;
        default:
            break;
    }
}
CINDER_APP( CosmicWebApp, RendererGl, []( App::Settings* settings ){
    settings->setWindowSize( ivec2( 1920, 1080 ) / 2 );
})
