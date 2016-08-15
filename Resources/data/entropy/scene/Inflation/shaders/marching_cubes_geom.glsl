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
uniform float fogMaxDistance;
uniform float fogMinDistance;
uniform float fogPower;

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

out vec4 rgba;

#define OUTPUT_NORMALS 1

#if OUTPUT_NORMALS
out vec3 normal;
#endif

layout (points) in;

#define WIREFRAME 1
#if WIREFRAME
layout (line_strip, max_vertices = 10) out;
const int vertexPerPrimitive = 2;
const float colorFactor = 0.3;
#else
layout (triangle_strip, max_vertices = 15) out;
const int vertexPerPrimitive = 3;
const float colorFactor = 0.5;
#endif

#define FOG_ENABLED 1

//Get vertex i position within current marching cube
vec3 cubePos(int i){
	return gl_in[0].gl_Position.xyz + vertDecals[i];
}
//Get vertex i value within current marching cube
vec4 cubeVal(int i){
    return texture(dataFieldTex, (cubePos(i)+vec3(0.5)));
}
//Get triangle table value
int triTableValue(int i, int j){
	return texelFetch(triTableTex, ivec2(j, i), 0).r;
}
//Compute interpolated vertex along an edge
vec3 vertexInterp(float isolevel, vec3 v0, float l0, vec3 v1, float l1){
	return mix(v0, v1, (isolevel-l0)/(l1-l0));
}
float floatInterp(float isolevel, float v0, float l0, float v1, float l1){
	return mix(v0, v1, (isolevel-l0)/(l1-l0));
}

float bright(vec3 rgb){
	return max(max(rgb.r, rgb.g), rgb.b);
}

float fog(float dist, float minDist, float maxDist, float power) {
	dist = pow(dist, power);
	minDist = pow(minDist, power);
	maxDist = pow(maxDist, power);
	float invDistanceToCamera = clamp(1 - (dist - minDist) / maxDist, 0.f, 1.f);
	if (dist > minDist) {
		return invDistanceToCamera;
	}
	else {
		return 1;
	}
}

//Geometry Shader entry point
void main(void) {

	/*if(cubeVal(0)>isolevel){
		gl_Position = modelViewProjectionMatrix * vec4(cubePos(0),1);
		EmitVertex();
	}
	EndPrimitive();
	return;*/

	int cubeindex=0;
    vec4 cubeVal0 = cubeVal(0);
    vec4 cubeVal1 = cubeVal(1);
    vec4 cubeVal2 = cubeVal(2);
    vec4 cubeVal3 = cubeVal(3);
    vec4 cubeVal4 = cubeVal(4);
    vec4 cubeVal5 = cubeVal(5);
    vec4 cubeVal6 = cubeVal(6);
    vec4 cubeVal7 = cubeVal(7);
	//Determine the index into the edge table which
	//tells us which vertices are inside of the surface
    cubeindex = int(cubeVal0.a < isolevel);
    cubeindex += int(cubeVal1.a < isolevel)*2;
    cubeindex += int(cubeVal2.a < isolevel)*4;
    cubeindex += int(cubeVal3.a < isolevel)*8;
    cubeindex += int(cubeVal4.a < isolevel)*16;
    cubeindex += int(cubeVal5.a < isolevel)*32;
    cubeindex += int(cubeVal6.a < isolevel)*64;
    cubeindex += int(cubeVal7.a < isolevel)*128;
	//Cube is entirely in/out of the surface
	if (cubeindex == 0 || cubeindex == 255){
		return;
	}

	vec3 vertlist[12];
	//Find the vertices where the surface intersects the cube
    vertlist[0] = vertexInterp(isolevel, cubePos(0), cubeVal0.a, cubePos(1), cubeVal1.a);
    vertlist[1] = vertexInterp(isolevel, cubePos(1), cubeVal1.a, cubePos(2), cubeVal2.a);
    vertlist[2] = vertexInterp(isolevel, cubePos(2), cubeVal2.a, cubePos(3), cubeVal3.a);
    vertlist[3] = vertexInterp(isolevel, cubePos(3), cubeVal3.a, cubePos(0), cubeVal0.a);
    vertlist[4] = vertexInterp(isolevel, cubePos(4), cubeVal4.a, cubePos(5), cubeVal5.a);
    vertlist[5] = vertexInterp(isolevel, cubePos(5), cubeVal5.a, cubePos(6), cubeVal6.a);
    vertlist[6] = vertexInterp(isolevel, cubePos(6), cubeVal6.a, cubePos(7), cubeVal7.a);
    vertlist[7] = vertexInterp(isolevel, cubePos(7), cubeVal7.a, cubePos(4), cubeVal4.a);
    vertlist[8] = vertexInterp(isolevel, cubePos(0), cubeVal0.a, cubePos(4), cubeVal4.a);
    vertlist[9] = vertexInterp(isolevel, cubePos(1), cubeVal1.a, cubePos(5), cubeVal5.a);
    vertlist[10] = vertexInterp(isolevel, cubePos(2), cubeVal2.a, cubePos(6), cubeVal6.a);
    vertlist[11] = vertexInterp(isolevel, cubePos(3), cubeVal3.a, cubePos(7), cubeVal7.a);

	// Create the triangle
	// gl_FrontColor=vec4(cos(isolevel*5.0-0.5), sin(isolevel*5.0-0.5), 0.5, 1.0);
	int i=0;
	//Strange bug with this way, uncomment to test
	//for (i=0; triTableValue(cubeindex, i)!=-1; i+=3) {
	while(i<16){
		if(triTableValue(cubeindex, i)!=-1){
			//Generate vertices of triangle//
			//Fill position varying attribute for fragment shader
            vec3 pos[vertexPerPrimitive];
            for(int j=0;j<vertexPerPrimitive;j++){
				int idx = triTableValue(cubeindex, i+j);
				pos[j] = vertlist[idx];
			}

            #if OUTPUT_NORMALS
                vec3 v1 = pos[1] - pos[0];
                vec3 v2 = pos[2] - pos[1];
                normal = normalize(vec3(
                    v1.y * v2.z - v1.z * v2.y,
                    v1.z * v2.x - v1.x * v2.z,
                    v1.x * v2.y - v1.y * v2.x
                ));
            #endif

			vec4 color = (cubeVal0 + cubeVal1 + cubeVal2 + cubeVal3 + cubeVal4 + cubeVal5 + cubeVal6 + cubeVal7) / 8.;
			#if WIREFRAME
				rgba = vec4(vec3(bright(color.rgb)), 0.25);
			#else
				rgba = color * colorFactor;
				rgba.a *= 0.5;
			#endif

			//Fill gl_Position attribute for vertex raster space position
            for(int j=0;j<vertexPerPrimitive;j++){
				vec4 eyePosition =  modelViewMatrix * vec4(pos[j],1.0f);
				gl_Position = projectionMatrix * eyePosition;
				#if FOG_ENABLED
				float distanceToCamera=length(eyePosition);
				rgba.a *= fog(distanceToCamera, fogMinDistance, fogMaxDistance, fogPower);
				#endif
				EmitVertex();
			}
			EndPrimitive();
		}else{
			break;
		}
		i=i+3; //Comment it for testing the strange bug
	}
}
