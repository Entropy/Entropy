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

		for(int z=-3;z<4;z++){
			for(int y=-3;y<4;y++){
				for(int x=-3;x<4;x++){
					if(x!=0 || y!=0 || z!=0){
						vec3 dir = normalize(vec3(x,y,z));
						acc += texelFetch(noiseField, noiseidx + ivec3(x,y,z), 0).a * dir;
					}
				}
			}
		}
		acc /= (7. * 7. * 7.);
		acc *= attractionForce;
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
}

