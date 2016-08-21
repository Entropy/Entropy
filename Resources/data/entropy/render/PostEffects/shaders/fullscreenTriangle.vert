#version 330 core

#pragma include <inc/ofDefaultUniforms.glsl>
#pragma include <inc/ofDefaultVertexInAttributes.glsl> 

#pragma include <inc/viewData.glsl>

out vec2 f_texcoord;

const vec2 triangleVerts[3] = vec2[](
  vec2( -1, -1 ),
  vec2(  3, -1 ),
  vec2( -1,  3 )
);

const vec2 nonFlippedTriangleTexCoords[3] = vec2[](
  vec2( 0, 0 ),
  vec2( 2, 0 ),
  vec2( 0, 2 )
);

const vec2 triangleTexCoords[3] = vec2[](
  vec2( 0, 1 ),
  vec2( 2, 1 ),
  vec2( 0, -1 )
);

void main( void )
{
   	f_texcoord = nonFlippedTriangleTexCoords[ gl_VertexID ];

	vec4 vertex = vec4( triangleVerts[ gl_VertexID ], 0.0f, 1.0f );
    gl_Position = vec4( vertex.xyz, 1.0f );
}
