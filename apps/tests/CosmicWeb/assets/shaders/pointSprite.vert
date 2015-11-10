#version 150

uniform mat4    ciModelViewProjection;
uniform mat4    ciModelView;
uniform float   uPointScale;
uniform float   uTime;
uniform float   uDensitySmoothing;
uniform float   uVelocityScale;

in vec4         ciPosition;
in vec4         aVelocity;
in vec4         aId;

out float       vDistance;
out float		vDensity;
out vec4		vId;

#define INSTANCED

void main(){
    vec4 position   = vec4( ( ciPosition.xyz - aVelocity.xyz * uTime * uVelocityScale ), 1.0 );//mix( finalPos, initialPos, uTime );

#ifdef INSTANCED
    int x = gl_InstanceID % 2;
    int y = ( gl_InstanceID - x ) / 2;
    position.xyz += vec3( 0, float(x), float(y) ) * vec3( 0, 120.8, 120.8 );
#endif

    vec4 eyeCoord 	= ciModelView * position;
    vDistance 		= length(eyeCoord);
    vDensity		= exp( -pow( ciPosition.w * 0.003, 2.0f ) / 2.0f / pow( uDensitySmoothing, 2.0f ) );
    vId 			= aId;
    gl_PointSize    = (uPointScale * 20.5 / vDistance);
    gl_Position     = ciModelViewProjection * position;
}