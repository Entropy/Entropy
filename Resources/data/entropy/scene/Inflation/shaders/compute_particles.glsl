#version 450
struct BlobVertex{
	vec4 pos;
	vec4 vel;
};

layout (std140, binding=0) buffer blob_positions
{
	BlobVertex vertices[];
};

vec3 repulsion(vec3 my_pos, vec3 their_pos){
	vec3 dir = my_pos-their_pos;
	float sqd = dot(dir,dir);
	if(sqd < 0.05*0.05){
		return (1./sqd)*dir;
	}
	return vec3(0.0);
}
//
// Description : Array and textureless GLSL 2D/3D/4D simplex
//               noise functions.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : stegu
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
//               https://github.com/stegu/webgl-noise
//

vec4 mod289(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0; }

float mod289(float x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0; }

vec4 permute(vec4 x) {
	 return mod289(((x*34.0)+1.0)*x);
}

float permute(float x) {
	 return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

float taylorInvSqrt(float r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

vec4 grad4(float j, vec4 ip)
  {
  const vec4 ones = vec4(1.0, 1.0, 1.0, -1.0);
  vec4 p,s;

  p.xyz = floor( fract (vec3(j) * ip.xyz) * 7.0) * ip.z - 1.0;
  p.w = 1.5 - dot(abs(p.xyz), ones.xyz);
  s = vec4(lessThan(p, vec4(0.0)));
  p.xyz = p.xyz + (s.xyz*2.0 - 1.0) * s.www;

  return p;
  }

// (sqrt(5) - 1)/4 = F4, used once below
const float F4 = 0.309016994374947451;

float snoise(vec4 v){
  const vec4  C = vec4( 0.138196601125011,  // (5 - sqrt(5))/20  G4
                        0.276393202250021,  // 2 * G4
                        0.414589803375032,  // 3 * G4
                       -0.447213595499958); // -1 + 4 * G4

// First corner
  vec4 i  = floor(v + dot(v, vec4(F4)) );
  vec4 x0 = v -   i + dot(i, C.xxxx);

// Other corners

// Rank sorting originally contributed by Bill Licea-Kane, AMD (formerly ATI)
  vec4 i0;
  vec3 isX = step( x0.yzw, x0.xxx );
  vec3 isYZ = step( x0.zww, x0.yyz );
//  i0.x = dot( isX, vec3( 1.0 ) );
  i0.x = isX.x + isX.y + isX.z;
  i0.yzw = 1.0 - isX;
//  i0.y += dot( isYZ.xy, vec2( 1.0 ) );
  i0.y += isYZ.x + isYZ.y;
  i0.zw += 1.0 - isYZ.xy;
  i0.z += isYZ.z;
  i0.w += 1.0 - isYZ.z;

  // i0 now contains the unique values 0,1,2,3 in each channel
  vec4 i3 = clamp( i0, 0.0, 1.0 );
  vec4 i2 = clamp( i0-1.0, 0.0, 1.0 );
  vec4 i1 = clamp( i0-2.0, 0.0, 1.0 );

  //  x0 = x0 - 0.0 + 0.0 * C.xxxx
  //  x1 = x0 - i1  + 1.0 * C.xxxx
  //  x2 = x0 - i2  + 2.0 * C.xxxx
  //  x3 = x0 - i3  + 3.0 * C.xxxx
  //  x4 = x0 - 1.0 + 4.0 * C.xxxx
  vec4 x1 = x0 - i1 + C.xxxx;
  vec4 x2 = x0 - i2 + C.yyyy;
  vec4 x3 = x0 - i3 + C.zzzz;
  vec4 x4 = x0 + C.wwww;

// Permutations
  i = mod289(i);
  float j0 = permute( permute( permute( permute(i.w) + i.z) + i.y) + i.x);
  vec4 j1 = permute( permute( permute( permute (
             i.w + vec4(i1.w, i2.w, i3.w, 1.0 ))
           + i.z + vec4(i1.z, i2.z, i3.z, 1.0 ))
           + i.y + vec4(i1.y, i2.y, i3.y, 1.0 ))
           + i.x + vec4(i1.x, i2.x, i3.x, 1.0 ));

// Gradients: 7x7x6 points over a cube, mapped onto a 4-cross polytope
// 7*7*6 = 294, which is close to the ring size 17*17 = 289.
  vec4 ip = vec4(1.0/294.0, 1.0/49.0, 1.0/7.0, 0.0) ;

  vec4 p0 = grad4(j0,   ip);
  vec4 p1 = grad4(j1.x, ip);
  vec4 p2 = grad4(j1.y, ip);
  vec4 p3 = grad4(j1.z, ip);
  vec4 p4 = grad4(j1.w, ip);

// Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;
  p4 *= taylorInvSqrt(dot(p4,p4));

// Mix contributions from the five corners
  vec3 m0 = max(0.6 - vec3(dot(x0,x0), dot(x1,x1), dot(x2,x2)), 0.0);
  vec2 m1 = max(0.6 - vec2(dot(x3,x3), dot(x4,x4)            ), 0.0);
  m0 = m0 * m0;
  m1 = m1 * m1;
  return 49.0 * ( dot(m0*m0, vec3( dot( p0, x0 ), dot( p1, x1 ), dot( p2, x2 )))
               + dot(m1*m1, vec2( dot( p3, x3 ), dot( p4, x4 ) ) ) ) ;

}

uniform float repulsionForce;
uniform float every;
uniform float now;
uniform float dt;
uniform sampler3D noiseField;
uniform float noiseSize;
uniform float bufferSize;
uniform float attractionForce;
uniform float frameNum;

layout(local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;
void main(void)
{
	if(gl_GlobalInvocationID.x*int(every)>bufferSize){
		return;
	}
	uint idx = gl_GlobalInvocationID.x * uint(every);
	vec3 pos = vertices[idx].pos.xyz;

	ivec3 noiseidx = ivec3((pos + vec3(0.5)) * noiseSize);
	vec3 acc = vec3(0);
	if((noiseidx.x > 4) && (noiseidx.x < (noiseSize - 4)) &&
	   (noiseidx.y > 4) && (noiseidx.y < (noiseSize - 4)) &&
	   (noiseidx.z > 4) && (noiseidx.z < (noiseSize - 4))){

		for(int z=-1;z<2;z++){
			for(int y=-1;y<2;y++){
				for(int x=-1;x<2;x++){
					if(x!=0 || y!=0 || z!=0){
						vec3 dir = normalize(vec3(x,y,z));
						acc += texelFetch(noiseField, noiseidx + ivec3(x,y,z), 0).a * dir;
					}
				}
			}
		}
		acc /= (3. * 3. * 3.);
		acc *= attractionForce;
		acc += vec3(1,1,1) * snoise(vec4(vec3(noiseidx) / 128., float(gl_GlobalInvocationID.x) / 1024.));
	}else{
		acc = -pos * attractionForce * 0.5;
	}
	int i=0;
	if(int(frameNum)%2==0){
		i = int(every);
	}
	for(;i<int(bufferSize);i+=int(every*2)){
		if(i!=idx){
			acc += repulsion(vertices[idx].pos.xyz, vertices[i].pos.xyz) * repulsionForce;
		}
	}

	vertices[idx].vel.xyz =  vertices[idx].vel.xyz * 0.9 + acc * 0.1 * dt;
	vertices[idx].pos.xyz += vertices[idx].vel.xyz;
	//vertices[idx].vel.xyz *= 0.9;

	if(pos.x>-0.001 && pos.x<0.001 && pos.y>-0.001 && pos.y<0.001 && pos.z>-0.001 && pos.z<0.001){
		vertices[idx].pos = vec4(10000,10000,10000,1);
	}
}

