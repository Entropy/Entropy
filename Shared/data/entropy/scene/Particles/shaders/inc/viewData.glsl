// Filename: viewData.glsl
// 
// Copyright © James Acres
// http://www.jamesacres.com
// http://github.com/jacres
// @jimmyacres
// 
// Created: Sat Nov  7 15:56:55 2015 (-0500)
// Last-Updated: Thu Nov 12 09:36:53 2015 (-0500)

#ifndef RTK_VIEW_DATA
#define RTK_VIEW_DATA

struct ViewData
{
	mat4  viewMatrix;
	mat4  inverseViewMatrix;
	vec2  viewportSize;
	vec2  rcpViewportSize;    
	float nearClip;
	float farClip;
};

layout (std140) uniform uViewBlock
{
	ViewData viewData;     
};

#endif // RTK_VIEW_DATA
