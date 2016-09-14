//
//  OpenCLImage3D.cpp
//  BubblesTest
//
//  Created by Elias Zananiri on 2016-03-03.
//
//

#include "OpenCLImage3D.h"

#if COMPUTE_CL_3D
namespace ent
{
    OpenCLImage3D::OpenCLImage3D()
    : OpenCLImage()
    {
        ofLog(OF_LOG_VERBOSE, "OpenCLImage3D::OpenCLImage3D");

        texture3d = nullptr;
    }

    // create a 3D Image from the ofxTexture3d passed in (they share memory space on device)
    // parameters with default values can be omited
    void OpenCLImage3D::initFromTexture3D(ofxTexture3d &tex3d,
                                          cl_mem_flags memFlags,
                                          int mipLevel)
    {
        ofLog(OF_LOG_VERBOSE, "OpenCLImage3D::initFromTexture3D");

        init(tex3d.texData.width, tex3d.texData.height, tex3d.texData.depth);

        cl_int err;
        if(clMemObject) clReleaseMemObject(clMemObject);

        clMemObject = clCreateFromGLTexture(pOpenCL->getContext(), memFlags, tex3d.texData.textureTarget, mipLevel, tex3d.texData.textureID, &err);
        assert(err != CL_INVALID_CONTEXT);
        assert(err != CL_INVALID_VALUE);
        //	assert(err != CL_INVALID_MIPLEVEL);
        assert(err != CL_INVALID_GL_OBJECT);
        assert(err != CL_INVALID_IMAGE_FORMAT_DESCRIPTOR);
        assert(err != CL_OUT_OF_HOST_MEMORY);
        assert(err == CL_SUCCESS);
        assert(clMemObject);
        
        texture3d = &tex3d;
        hasCorrespondingGLObject = true;
    }

    // create both a 3D Image AND an ofxTexture3d at the same time (they share memory space on device)
    // parameters with default values can be omitted
    void OpenCLImage3D::initWithTexture3D(int width,
                                          int height,
                                          int depth,
                                          int glTypeInternal,
                                          cl_mem_flags memFlags)
    {
        ofLog(OF_LOG_VERBOSE, "OpenCLImage3D::initWithTexture3D");

        if (texture3d) delete texture3d;
        texture3d = new ofxTexture3d();
        texture3d->allocate(width, height, depth, glTypeInternal);
        initFromTexture3D(*texture3d, memFlags, 0);
        reset3D();
    }

    // return reference to related ofxTexture3d
    // this may be NULL if no ofTexture was setup
    ofxTexture3d& OpenCLImage3D::getTexture3D()
    {
        return *texture3d;
    }

    void OpenCLImage3D::reset3D()
    {
        ofLog(OF_LOG_VERBOSE, "OpenCLImage3D::reset3D");

        size_t numElements = width * height * depth * 4; // TODO, make real
        if (ofGetGlTypeFromInternal(texture3d->texData.glInternalFormat) == GL_FLOAT) {
            numElements *= sizeof(cl_float);
        }
        char *data = new char[numElements];
        memset(data, 0, numElements);
        write(data, true);
        delete [] data;
    }
}
#endif
