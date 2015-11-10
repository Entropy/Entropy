#version 150

uniform mat4    ciModelViewProjection;
uniform mat4    ciModelView;
uniform float   uPointScale;
uniform float   uTime;
in vec4         ciPosition;
in vec4         aInitialPosition;
out float       vDistance;
out float       vBubbleFlag;

void main(){
    vec4 initialPos = vec4( aInitialPosition.xyz * 0.1, 1.0 );
    vec4 finalPos   = vec4( ciPosition.xyz * 0.1, 1.0 );
    vec4 position   = mix( finalPos, initialPos, uTime );
    vec4 eyeCoord 	= ciModelView * position;
    vDistance 		= length(eyeCoord);
    vBubbleFlag     = ciPosition.w;
    gl_PointSize    = mix( 2.5f, 7.5f, vBubbleFlag ) * (uPointScale * 0.4 / vDistance);
    gl_Position     = ciModelViewProjection * position;
}