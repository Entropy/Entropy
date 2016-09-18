//
//  ofxGaussianMapTexture.h
//  PartyCL
//
//  Created by Elias Zananiri on 2016-01-28.
//
//

#pragma once

#include "ofTexture.h"

namespace entropy
{
	namespace surveys
	{
		void CreateGaussianMapTexture(ofTexture & texture, int resolution, int textureTarget = (ofGetUsingArbTex() ? GL_TEXTURE_RECTANGLE_ARB : GL_TEXTURE_2D));
	}
}
