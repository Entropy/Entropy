//
//  ofxGaussianMapTexture.cpp
//  PartyCL
//
//  Created by Elias Zananiri on 2016-01-28.
//
//

#include "ofxGaussianMapTexture.h"

//--------------------------------------------------------------
inline float evalHermite(float pA, float pB, float vA, float vB, float u)
{
    float u2=(u*u), u3=u2*u;
    float b0 = 2*u3 - 3*u2 + 1;
    float b1 = -2*u3 + 3*u2;
    float b2 = u3 - 2*u2 + u;
    float b3 = u3 - u;
    return( b0*pA + b1*pB + b2*vA + b3*vB );
}

//--------------------------------------------------------------
unsigned char* createGaussianMap(int N)
{
    float *M = new float[2*N*N];
    unsigned char *B = new unsigned char[4*N*N];
    float X,Y,Y2,Dist;
    float Incr = 2.0f/N;
    int i=0;
    int j = 0;
    Y = -1.0f;
    //float mmax = 0;
    for (int y=0; y<N; y++, Y+=Incr)
    {
        Y2=Y*Y;
        X = -1.0f;
        for (int x=0; x<N; x++, X+=Incr, i+=2, j+=4)
        {
            Dist = (float)sqrtf(X*X+Y2);
            if (Dist>1) Dist=1;
            M[i+1] = M[i] = evalHermite(1.0f,0,0,0,Dist);
            B[j+3] = B[j+2] = B[j+1] = B[j] = (unsigned char)(M[i] * 255);
        }
    }
    delete [] M;
    return(B);
}

//--------------------------------------------------------------
void ofxCreateGaussianMapTexture(ofTexture& texture, int resolution, int textureTarget)
{
    ofTextureData textureData;
    textureData.width = resolution;
    textureData.height = resolution;
    textureData.glInternalFormat = GL_RGBA;
    textureData.textureTarget = textureTarget;

    unsigned char *data = createGaussianMap(resolution);

    texture.allocate(textureData);
    texture.loadData(data, resolution, resolution, GL_RGBA);
}
