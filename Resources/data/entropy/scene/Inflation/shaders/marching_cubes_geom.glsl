#version 450
uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
//Volume data field texture
uniform sampler3D dataFieldTex;
//Triangles table texture
uniform isampler2D triTableTex;
//Global iso level
uniform float isolevel;

const float resolution = 128.f;

//Marching cubes vertices decal
const vec3 vertDecals[8] = {
    vec3(0.0f, 0.0f, 0.0f)/resolution,
    vec3(1.0f, 0.0f, 0.0f)/resolution,
    vec3(1.0f, 1.0f, 0.0f)/resolution,
    vec3(0.0f, 1.0f, 0.0f)/resolution,
    vec3(0.0f, 0.0f, 1.0f)/resolution,
    vec3(1.0f, 0.0f, 1.0f)/resolution,
    vec3(1.0f, 1.0f, 1.0f)/resolution,
    vec3(0.0f, 1.0f, 1.0f)/resolution,
};

#define OUTPUT_NORMALS 0
#define SUBDIVISIONS 1

out vec4 position;
out vec4 color;
#if OUTPUT_NORMALS
out vec3 normal;
#endif



layout (points) in;
#if SUBDIVISIONS
layout (triangle_strip, max_vertices = 15*SUBDIVISIONS*3) out;
#else
layout (triangle_strip, max_vertices = 15) out;
#endif

//Get vertex i position within current marching cube
vec3 cubePos(int i){
	return gl_in[0].gl_Position.xyz + vertDecals[i];
}

//Get vertex i value within current marching cube
vec4 cubeVal(int i){
    return texture(dataFieldTex, (cubePos(i)+vec3(0.5)));
}

vec4 cubeVal(vec3 i){
	return texture(dataFieldTex, (i+vec3(0.5)));
}

//Get triangle table value
int triTableValue(int i, int j){
	return texelFetch(triTableTex, ivec2(j, i), 0).r;
}

//Compute interpolated vertex along an edge
vec3 vertexInterp(float isolevel, vec3 v0, float l0, vec3 v1, float l1){
	return mix(v0, v1, (isolevel-l0)/(l1-l0));
}

vec4 vertexInterp(float isolevel, vec4 v0, float l0, vec4 v1, float l1){
	return mix(v0, v1, (isolevel-l0)/(l1-l0));
}

float floatInterp(float isolevel, float v0, float l0, float v1, float l1){
	return mix(v0, v1, (isolevel-l0)/(l1-l0));
}

int index1[12] = {0,1,2,3,4,5,6,7,0,1,2,3};
int index2[12] = {1,2,3,0,5,6,7,4,4,5,6,7};

//Geometry Shader entry point
void main(void) {
	vec3 cubePoss[8];
	cubePoss[0] = cubePos(0);
	cubePoss[1] = cubePos(1);
	cubePoss[2] = cubePos(2);
	cubePoss[3] = cubePos(3);
	cubePoss[4] = cubePos(4);
	cubePoss[5] = cubePos(5);
	cubePoss[6] = cubePos(6);
	cubePoss[7] = cubePos(7);

	int cubeindex=0;
	vec4 cubeVals[8];
	cubeVals[0] = cubeVal(cubePoss[0]);
	cubeVals[1] = cubeVal(cubePoss[1]);
	cubeVals[2] = cubeVal(cubePoss[2]);
	cubeVals[3] = cubeVal(cubePoss[3]);
	cubeVals[4] = cubeVal(cubePoss[4]);
	cubeVals[5] = cubeVal(cubePoss[5]);
	cubeVals[6] = cubeVal(cubePoss[6]);
	cubeVals[7] = cubeVal(cubePoss[7]);

	//Determine the index into the edge table which
	//tells us which vertices are inside of the surface
	cubeindex = int(cubeVals[0].a < isolevel);
	cubeindex += int(cubeVals[1].a < isolevel)*2;
	cubeindex += int(cubeVals[2].a < isolevel)*4;
	cubeindex += int(cubeVals[3].a < isolevel)*8;
	cubeindex += int(cubeVals[4].a < isolevel)*16;
	cubeindex += int(cubeVals[5].a < isolevel)*32;
	cubeindex += int(cubeVals[6].a < isolevel)*64;
	cubeindex += int(cubeVals[7].a < isolevel)*128;
	//Cube is entirely in/out of the surface
	if (cubeindex == 0 || cubeindex == 255){
		return;
	}

	int i=0;
	for (i=0; triTableValue(cubeindex, i)!=-1; i+=3) {
		vec3 pos[3];
		color = vec4(0);
		for(int j=0; j<3; j++){
			int idx = triTableValue(cubeindex, i+j);
			//pos[j] = vertlist[idx];
			pos[j] = vertexInterp(isolevel, cubePoss[index1[idx]], cubeVals[index1[idx]].a, cubePoss[index2[idx]], cubeVals[index2[idx]].a);
			color += vertexInterp(isolevel, cubeVals[index1[idx]], cubeVals[index1[idx]].a, cubeVals[index2[idx]], cubeVals[index2[idx]].a);
		}
		color /= 3.;
		//color = (cubeVal0 + cubeVal1 + cubeVal2 + cubeVal3 + cubeVal4 + cubeVal5 + cubeVal6 + cubeVal7) / 8.;

        #if OUTPUT_NORMALS
		    vec3 v1 = pos[1] - pos[0];
			vec3 v2 = pos[2] - pos[1];
			normal = normalize(vec3(
			    v1.y * v2.z - v1.z * v2.y,
			    v1.z * v2.x - v1.x * v2.z,
			    v1.x * v2.y - v1.y * v2.x
			));
        #endif

        #if SUBDIVISIONS
			vec3 center = (pos[0] + pos[1] + pos[2]) / 3.;
			position = vec4(pos[0], 1.0);
			EmitVertex();
			position = vec4(center, 1.0);
			EmitVertex();
			position = vec4(pos[1], 1.0);
			EmitVertex();
			EndPrimitive();

			position = vec4(pos[1], 1.0);
			EmitVertex();
			position = vec4(center, 1.0);
			EmitVertex();
			position = vec4(pos[2], 1.0);
			EmitVertex();
			EndPrimitive();

			position = vec4(pos[2], 1.0);
			EmitVertex();
			position = vec4(center, 1.0);
			EmitVertex();
			position = vec4(pos[0], 1.0);
			EmitVertex();
			EndPrimitive();
        #else
			for(int j=0; j<3; j++){
				position = vec4(pos[j], 1.0);
				EmitVertex();
			}
			EndPrimitive();
        #endif
	}
}
