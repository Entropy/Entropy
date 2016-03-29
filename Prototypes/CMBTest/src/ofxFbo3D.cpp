//
//  ofxFbo3d.cpp
//  ofxVolumetricsExample
//
//  Created by Elias Zananiri on 2016-03-09.
//  https://devtalk.nvidia.com/default/topic/818946/opengl/render-to-3d-texture/
//

#include "ofxFbo3D.h"
#include "GLError.h"

//----------------------------------------------------------
ofxFbo3D::ofxFbo3D()
    : _fboID(0)
{

}

//----------------------------------------------------------
ofxFbo3D::~ofxFbo3D()
{
    clear();
}

//----------------------------------------------------------
void ofxFbo3D::allocate()
{
    clear();
    
    glGenFramebuffers(1, &_fboID);
    cout << "Allocating fboID " << _fboID << endl;
    lb::CheckGLError();
}

//----------------------------------------------------------
void ofxFbo3D::clear()
{
    if (_fboID) {
        cout << "Clearing fboID " << _fboID << endl;
        glDeleteFramebuffers(1, &_fboID);
        _fboID = 0;
    }
    _mrt.clear();
}

//----------------------------------------------------------
bool ofxFbo3D::checkStatus() const
{
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    switch (status) {
    case GL_FRAMEBUFFER_COMPLETE:
        ofLogVerbose("ofFbo") << "FRAMEBUFFER_COMPLETE - OK";
        return true;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        ofLogError("ofFbo") << "FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        ofLogError("ofFbo") << "FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
        ofLogError("ofFbo") << "FRAMEBUFFER_INCOMPLETE_DIMENSIONS";
        break;
    case GL_FRAMEBUFFER_UNSUPPORTED:
        ofLogError("ofFbo") << "FRAMEBUFFER_UNSUPPORTED";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        ofLogWarning("ofFbo") << "FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        ofLogError("ofFbo") << "FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
        ofLogError("ofFbo") << "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
        break;
    default:
        ofLogError("ofFbo") << "UNKNOWN ERROR " << status;
        break;

    }

    return false;
}

//----------------------------------------------------------
void ofxFbo3D::attachTexture(ofTexture& texture, GLenum attachmentPoint)
{
    attachTexture(texture.texData.textureID, attachmentPoint);
}

//----------------------------------------------------------
void ofxFbo3D::attachTexture(ofxTexture3d& texture, GLenum attachmentPoint)
{
    attachTexture(texture.getTextureData().textureID, attachmentPoint);
}

//----------------------------------------------------------
void ofxFbo3D::attachTexture(GLuint textureID, GLenum attachmentPoint)
{
    // bind fbo for textures (if using MSAA this is the newly created fbo, otherwise its the same fbo as before)
    GLint temp;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &temp);
    glBindFramebuffer(GL_FRAMEBUFFER, _fboID);

    cout << "Attaching textureID " << textureID << endl;
    lb::CheckGLError();

    GLenum attachment = GL_COLOR_ATTACHMENT0 + attachmentPoint;
    glFramebufferTexture(GL_FRAMEBUFFER, attachment, textureID, 0);
    if (attachmentPoint >= _mrt.size()) {
        _mrt.resize(attachmentPoint + 1);
    }
    _mrt[attachmentPoint] = attachment;
    lb::CheckGLError();

    glBindFramebuffer(GL_FRAMEBUFFER, temp);
}

//----------------------------------------------------------
void ofxFbo3D::bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, _fboID);
}

//----------------------------------------------------------
void ofxFbo3D::unbind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//----------------------------------------------------------
void ofxFbo3D::setLayer(GLenum attachment, GLuint textureID, GLuint layer)
{
    glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment, textureID, 0, layer);
}

//----------------------------------------------------------
void ofxFbo3D::activateDrawBuffers()
{
    glDrawBuffers(_mrt.size(), _mrt.data());
}

//----------------------------------------------------------
void ofxFbo3D::begin()
{
    ofPushView();
    ofPushStyle();
    ofViewport();
    ofSetupScreenPerspective();
    bind();
    lb::CheckGLError();

    activateDrawBuffers();
    lb::CheckGLError();
}

//----------------------------------------------------------
void ofxFbo3D::end()
{
    unbind();
    lb::CheckGLError();

    ofPopStyle();
    ofPopView();
}

//----------------------------------------------------------
GLuint ofxFbo3D::getFboID() const
{
    return _fboID;
}
