#version 330

in vec3 position;
in vec2 texcoord;
uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform vec4 viewport;
uniform vec4 billboard_position;
out vec2 v_texcoord;
flat out int v_valid;

void main(void){
	vec4 dScreen4 = modelViewProjectionMatrix * billboard_position;
	vec3 dScreen = dScreen4.xyz / dScreen4.w;
	dScreen += vec3(1.0) ;
	dScreen *= 0.5;

	dScreen.x += viewport.x;
	dScreen.x *= viewport.z;

	dScreen.y += viewport.y;
	dScreen.y *= viewport.w;

	if (dScreen.z >= 1 || dScreen.z<=0){
		v_valid = 0;
		gl_Position = vec4(0);
	}else{
		v_valid = 1;
		vec3 pos = position;
		pos += vec3(dScreen.x, dScreen.y, 0);
		pos *= vec3(2/viewport.z, 2/viewport.w, 1);
		pos += vec3(-1,-1,0);

		float far=gl_DepthRange.far;
		float near=gl_DepthRange.near;

		vec4 clip_space_pos = modelViewProjectionMatrix * vec4(position + billboard_position.xyz,1);
		float ndc_depth = clip_space_pos.z / clip_space_pos.w;

		gl_Position = vec4(pos.xy, ndc_depth, 1.0);
		v_texcoord = texcoord;
	}
}
