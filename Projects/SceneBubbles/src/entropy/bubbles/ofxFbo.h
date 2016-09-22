//
//  ofxFbo.h
//  ofxVolumetricsExample
//
//  Created by Elias Zananiri on 2016-03-09.
//
//

#pragma once

#include "ofxTexture.h"

class ofxFbo
{
public:
	ofxFbo();
    ~ofxFbo();

    void allocate();
    void clear();
    bool checkStatus() const;

	void attachTexture(ofTexture& texture, GLenum attachmentPoint);
	void attachTexture(ofxTexture& texture, GLenum attachmentPoint);
	void attachTexture(GLuint textureID, GLenum attachmentPoint);

    void bind() const;
    void unbind() const;

    void activateDrawBuffers();
    void setLayer(GLenum attachment, GLuint textureID, GLuint layer);

    void begin();
    void end();

    GLuint getFboID() const;

private:
    GLuint _fboID;
    GLuint _depthBufferID;

    vector<GLenum> _mrt;
};