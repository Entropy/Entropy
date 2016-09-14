//
//  OpenCLImage3D.h
//  BubblesTest
//
//  Created by Elias Zananiri on 2016-03-03.
//
//

#pragma once
#include "Constants.h"

#if COMPUTE_CL_3D
#include "MSAOpenCL.h"
#include "ofxTexture3d.h"

namespace ent
{
    class OpenCLImage3D
    : public msa::OpenCLImage
    {
    public:
        OpenCLImage3D();

        // create a 3D Image from the ofxTexture3d passed in (they share memory space on device)
        // parameters with default values can be omited
        void initFromTexture3D(ofxTexture3d &tex,
                               cl_mem_flags memFlags = CL_MEM_READ_WRITE,
                               int mipLevel = 0);



        // create both a 3D Image AND an ofxTexture3d at the same time (they share memory space on device)
        // parameters with default values can be omited
        void initWithTexture3D(int width,
                               int height,
                               int depth,
                               int glTypeInternal = GL_RGBA,
                               cl_mem_flags memFlags = CL_MEM_READ_WRITE);

        // return reference to related ofxTexture3d
        // this may be NULL if no ofTexture was setup
        ofxTexture3d& getTexture3D();

        void reset3D();

    protected:
        ofxTexture3d *texture3d;
    };
}
#endif
