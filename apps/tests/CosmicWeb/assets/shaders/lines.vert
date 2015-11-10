#version 150

uniform mat4    ciModelViewProjection;
uniform mat4    ciModelView;

in vec4         ciPosition;
out float       vDistance;

void main(){
    vec4 position   = vec4( ciPosition.xyz * 0.1, 1.0 );
    vec4 eyeCoord 	= ciModelView * position;
    vDistance 		= length(eyeCoord);
    gl_Position     = ciModelViewProjection * position;
}