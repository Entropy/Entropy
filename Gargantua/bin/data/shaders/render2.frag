#version 150

// Based on Gargantua by jun
// https://www.shadertoy.com/view/ldjSDc

#define PI 3.14159

uniform vec3      iResolution;           // viewport resolution (in pixels)
uniform float     iGlobalTime;           // shader playback time (in seconds)
uniform float     iTimeDelta;            // render time (in seconds)
uniform int       iFrame;                // shader playback frame
uniform float     iChannelTime[2];       // channel playback time (in seconds)
uniform vec3      iChannelResolution[2]; // channel resolution (in pixels)
uniform vec4      iMouse;                // mouse pixel coords. xy: current (if MLB down), zw: click
uniform sampler2D iChannel0;             // input channel. XX = 2D/Cube
uniform sampler2D iChannel1;             // input channel. XX = 2D/Cube
uniform vec4      iDate;                 // (year, month, day, time in seconds)
uniform float     iSampleRate;           // sound sample rate (i.e., 44100)

uniform vec4 globalColor;

struct Camera
{
    vec3 position;
    vec3 upDir;
    mat4 orientation;
    float fov;
};
uniform Camera uCamera;

struct BlackHole
{
    vec3 position;
    float radius;
    float mass;
    float lensing;
    float speed;
    float ringThickness;
    vec4 color;
};
uniform BlackHole uBlackHole;

uniform float uColorMix;
uniform float uColorBlowOut;

in vec2 vTexCoord;

out vec4 fFragColor;

/*

 Black hole with gravitational lensing and accretion disc.

 Reference:
 Bozza, Valerio. "Gravitational lensing by black holes." General Relativity and Gravitation 42.9 (2010): 2269-2300.

 Jun 2014.11.24

 */


float r_bar;//=2.7*1.2;//uBlackHole.mass;
float disc_r_orig;//=r_bar * 2.1;
float disc_r;//=disc_r_orig;

vec3 disc_n=vec3(0.,1.,0.);
vec3 disc_s=vec3(1.,0.,0.);
//vec3 eye=vec3(0.,0.4,10.);
//vec3 up=normalize(vec3(0.08,1.,0.));
//mat3 cam_mat;// camera -> world
//float tan_half_vfov=1.0;
//vec2 iplane_size=2.*tan_half_vfov*vec2(iResolution.x/iResolution.y,1.);

vec3 rot_x(vec3 v,float theta)
{
    float s=sin(theta),c=cos(theta);
    return vec3(v.x,c*v.y-s*v.z,s*v.y+c*v.z);
}
vec3 rot_y(vec3 v,float theta)
{
    float s=sin(theta),c=cos(theta);
    return vec3(c*v.x+s*v.z,v.y,c*v.z-s*v.x);
}
vec3 rot_z(vec3 v,float theta)
{
    float s=sin(theta),c=cos(theta);
    return vec3(c*v.x-s*v.y,s*v.x+c*v.y,v.z);
}

vec4 discColor(vec3 pos)
{
    vec3 v = pos - uBlackHole.position;
    float d = length(v);
    v /= d;
    if (d < disc_r) {
        vec2 uv = vec2((atan(dot(v,disc_s),dot(v,cross(disc_s,disc_n)))/(2.*PI)*1.-iGlobalTime*uBlackHole.speed),
                     (d-r_bar)/(disc_r-r_bar));
        return 3.*texture(iChannel1,uv)*smoothstep(disc_r,r_bar,d);
    }
    else return vec4(0.);
}

void main()
{
    r_bar = uBlackHole.radius * uBlackHole.mass;
    disc_r_orig=r_bar * uBlackHole.ringThickness;
    disc_r=disc_r_orig;

    float ruv = length((vTexCoord-0.5*iResolution.xy)/iResolution.y);
//    vec4 color= mix(uColorOuter, uColorInner, exp(ruv * uColorMix) * 1.15);
//    vec4 color=vec4(0.7,0.6,0.8,1.) * exp(-ruv*1.1)*1.15;
    vec4 color = globalColor * exp(ruv * uColorMix) * uColorBlowOut;

    vec2 planeSize = tan(uCamera.fov) * vec2(iResolution.x/iResolution.y, 1.0);
    vec2 ixy = (vTexCoord / iResolution.xy - 0.5) * planeSize;

    mat3 camMatrix = mat3(uCamera.orientation);

    vec3 ray_dir=camMatrix * normalize(vec3(ixy,-1.));
    vec3 h2e = uCamera.position - uBlackHole.position;
    float l2_h2e=dot(h2e,h2e);
    float rm=length(cross(ray_dir,h2e)); // smallest distance
    float t_cp=sqrt(l2_h2e-rm*rm); // t of closest point

    float alpha = 0.0;
    if (uBlackHole.lensing > 0.0) {
        alpha = uBlackHole.lensing * uBlackHole.mass / rm;
        disc_r=disc_r_orig;
        if (rm < r_bar) {  // hack
            alpha *= (1.0 - abs(dot(disc_n, uCamera.upDir)));
            disc_r*=1.25;
        }
    }

    float tan_a_2=tan(alpha*0.5);

    vec3 cp = uCamera.position + ray_dir*t_cp;// closest point
    vec3 coord_origin=cp+ray_dir*(rm*tan_a_2);
    vec3 x_axis=normalize(uBlackHole.position - coord_origin);
    vec3 y_axis=normalize(ray_dir+tan_a_2*normalize(uBlackHole.position-cp));
    vec3 z_axis=cross(x_axis,y_axis);

    float c=length(uBlackHole.position-coord_origin);
    float k=tan_a_2; // a/b

    // the intersection line pass through bh
    vec3 iline_r=normalize(cross(z_axis,disc_n));

    float x1=-1.,x2=-1.,y1,y2;

    if (uBlackHole.lensing > 0.0) {
        float k2=k*k;
        float b2=c*c/(1.0+k2);
        float a2=k2*b2;
        float a=sqrt(a2);
        // x^2/a2 - y^2/b2 = 1

        float denom=dot(x_axis,iline_r);
        if(denom==0.)
        {
            x1=x2=c;
            y1=-b2/sqrt(a2);
            y2=-y1;
        }
        else
        {
            float slope=dot(y_axis,iline_r)/denom; // y=slope*(x-c)
            k2=slope*slope; // override k2
            float A=a2*k2-b2;
            float B=-2.*a2*k2*c;
            float C=a2*(k2*c*c+b2);
            // B*B-4AC>=0
            float delta=sqrt(B*B-4.*A*C);
            x1=(-B-delta)/(2.*A);
            x2=(-B+delta)/(2.*A);
            y1=slope*(x1-c);
            y2=slope*(x2-c);
        }
    }
    else {
        float denom=dot(x_axis,iline_r);
        if(denom!=0.) // else no intersection
        {
            float slope=dot(y_axis,iline_r)/denom;
            x1=0.;y1=-slope*c;
        }
    }

    vec3 o2e= uCamera.position - coord_origin;
    float yeye=dot(o2e,y_axis);
    vec3 p1=coord_origin+x1*x_axis+y1*y_axis,
    p2=coord_origin+x2*x_axis+y2*y_axis;
    if(x1>=0.&&y1>=yeye && ((y1<0.&&length(p1-uBlackHole.position)>r_bar)||rm>r_bar))
    {
        color += discColor(p1);
    }
    if(x2>=0.&&y2>=yeye && ((y2<0.&&length(p2-uBlackHole.position)>r_bar) ||rm>r_bar))
    {
        color += discColor(p2);
    }

    fFragColor = color;
}
