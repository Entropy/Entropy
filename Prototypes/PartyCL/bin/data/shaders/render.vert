#version 150

// OF default uniforms and attributes
uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;

in vec4 position;

// App uniforms and attributes
uniform float uPointSize;

void main()                                                  
{                                                            
    vec4 vertex = vec4(position.rgb, 1.0);
    vec4 eyeCoord = modelViewMatrix * vertex;
    gl_Position = projectionMatrix * eyeCoord;

    float attenuation = uPointSize / length(eyeCoord);

//    float pointSize = uPointSize;
    gl_PointSize = uPointSize * attenuation; //max(1.0, pointSize / (1.0 - eyeCoord.z));


//    float dist = sqrt(eyeCoord.x * eyeCoord.x + eyeCoord.y * eyeCoord.y + eyeCoord.z * eyeCoord.z);
//    float attenuation = 600.0 / dist;
//
//    //	gl_PointSize = normal.x * attenuation;
//    gl_PointSize = pointSize * attenuation;
//
//    vType = 1.0; //normal.y;
//
//    float sat = clamp(mix(1.0, 0.0, attenuation), 0.0, 1.0);
//    vColor = vec4(hsv2rgb(vec3(0.0, sat, 1.0)), 1.0);


//    float pointSize = 500.0 * gl_Point.size;
//    vec4 vert = gl_Vertex;
//    vert.w = 1.0;
//    vec3 pos_eye = vec3 (gl_ModelViewMatrix * vert);
//    gl_PointSize = max(1.0, pointSize / (1.0 - pos_eye.z));
//    gl_TexCoord[0] = gl_MultiTexCoord0;
//    //    gl_TexCoord[1] = gl_MultiTexCoord1;
//    gl_Position = ftransform();
//    gl_FrontColor = gl_Color;
}
