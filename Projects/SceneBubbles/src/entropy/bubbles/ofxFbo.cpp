//
//  ofxFbo.cpp
//  ofxVolumetricsExample
//
//  Created by Elias Zananiri on 2016-03-09.
//  https://devtalk.nvidia.com/default/topic/818946/opengl/render-to-3d-texture/
//

#include "ofxFbo.h"

//----------------------------------------------------------
ofxFbo::ofxFbo()
    : _fboID(0)
{

}

//----------------------------------------------------------
ofxFbo::~ofxFbo()
{
    clear();
}

//----------------------------------------------------------
void ofxFbo::allocate()
{
    clear();
    
    glGenFramebuffers(1, &_fboID);
}

//----------------------------------------------------------
void ofxFbo::clear()
{
    if (_fboID) {
        glDeleteFramebuffers(1, &_fboID);
        _fboID = 0;
    }
    _mrt.clear();
}

//----------------------------------------------------------
bool ofxFbo::checkStatus() const
{
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    switch (status) {
    case GL_FRAMEBUFFER_COMPLETE:
        ofLogVerbose(__FUNCTION__) << "FRAMEBUFFER_COMPLETE - OK";
        return true;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        ofLogError(__FUNCTION__) << "FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        ofLogError(__FUNCTION__) << "FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
        ofLogError(__FUNCTION__) << "FRAMEBUFFER_INCOMPLETE_DIMENSIONS";
        break;
    case GL_FRAMEBUFFER_UNSUPPORTED:
        ofLogError(__FUNCTION__) << "FRAMEBUFFER_UNSUPPORTED";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        ofLogWarning(__FUNCTION__) << "FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        ofLogError(__FUNCTION__) << "FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
        ofLogError(__FUNCTION__) << "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
        break;
    default:
        ofLogError(__FUNCTION__) << "UNKNOWN ERROR " << status;
        break;

    }

    return false;
}

//----------------------------------------------------------
void ofxFbo::attachTexture(ofTexture& texture, GLenum attachmentPoint)
{
    attachTexture(texture.texData.textureID, attachmentPoint);
}

//----------------------------------------------------------
void ofxFbo::attachTexture(ofxTexture& texture, GLenum attachmentPoint)
{
	attachTexture(texture.texData.textureID, attachmentPoint);
}

//----------------------------------------------------------
void ofxFbo::attachTexture(GLuint textureID, GLenum attachmentPoint)
{
    // bind fbo for textures (if using MSAA this is the newly created fbo, otherwise its the same fbo as before)
    GLint temp;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &temp);
    glBindFramebuffer(GL_FRAMEBUFFER, _fboID);

    GLenum attachment = GL_COLOR_ATTACHMENT0 + attachmentPoint;
    glFramebufferTexture(GL_FRAMEBUFFER, attachment, textureID, 0);
    if (attachmentPoint >= _mrt.size()) {
        _mrt.resize(attachmentPoint + 1);
    }
    _mrt[attachmentPoint] = attachment;

    glBindFramebuffer(GL_FRAMEBUFFER, temp);
}

//----------------------------------------------------------
void ofxFbo::bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, _fboID);
}

//----------------------------------------------------------
void ofxFbo::unbind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//----------------------------------------------------------
void ofxFbo::setLayer(GLenum attachment, GLuint textureID, GLuint layer)
{
    glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment, textureID, 0, layer);
}

//----------------------------------------------------------
void ofxFbo::activateDrawBuffers()
{
    glDrawBuffers(_mrt.size(), _mrt.data());
}

//----------------------------------------------------------
void ofxFbo::begin()
{
    //ofPushView();
    //ofPushStyle();
    //ofViewport();
    //ofSetupScreenPerspective();
    bind();

    activateDrawBuffers();
}

//----------------------------------------------------------
void ofxFbo::end()
{
    unbind();

    //ofPopStyle();
    //ofPopView();
}

//----------------------------------------------------------
GLuint ofxFbo::getFboID() const
{
    return _fboID;
}
