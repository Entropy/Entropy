#version 330

// OF default uniforms and attributes
uniform vec4 globalColor;

// App uniforms and attributes
uniform sampler3D volume_tex;
uniform vec3 vol_d;
uniform vec3 vol_tex_d;
uniform float zoffset;
uniform float quality;
uniform float threshold;
uniform float density;

in vec4 vPosition;
in vec3 vTexCoord;
in vec3 vCameraPosition;

out vec4 fragColor;

const float levelThreshold = 0.00001;

struct Ray 
{
	vec3 Origin;
	vec3 Dir;
};

struct BoundingBox 
{
	vec3 Min;
	vec3 Max;
};

float luminance(vec3 rgb)
{
	return dot(vec3(0.2126729, 0.7151522, 0.0721750), rgb);
}

bool intersectBox(Ray ray, BoundingBox box, out float t0, out float t1)
{
	vec3 invR = 1.0 / ray.Dir;
	vec3 tbot = invR * (box.Min-ray.Origin);
	vec3 ttop = invR * (box.Max-ray.Origin);
	vec3 tmin = min(ttop, tbot);
	vec3 tmax = max(ttop, tbot);
	vec2 t = max(tmin.xx, tmin.yz);
	t0 = max(t.x, t.y);
	t = min(tmax.xx, tmax.yz);
	t1 = min(t.x, t.y);
	return t0 <= t1;
}

void main()
{
	vec3 minv = vec3(0.)+1./vol_tex_d;
	vec3 maxv = (vol_d/vol_tex_d)-1./vol_tex_d;
	vec3 vec;
	vec3 vold = (maxv-minv)*vol_d;
	float vol_l = length(vold);

	//vec4 col_acc = vec4(vec3(0.9),0);
	vec4 col_acc = vec4(0.0);
	vec3 zOffsetVec = vec3(0.0,0.0,zoffset/vol_tex_d.z);
	vec3 backPos = vTexCoord;
	vec3 lookVec = normalize(backPos - vCameraPosition);


	Ray eye = Ray(vCameraPosition, lookVec);
	BoundingBox box = BoundingBox(vec3(0.),vec3(1.));

	float tnear;
	float tfar;
	intersectBox(eye, box, tnear, tfar);
	if(tnear < 0.15) tnear = 0.15;
	if(tnear > tfar) discard;

	vec3 rayStart = (eye.Origin + eye.Dir * tnear)*(maxv-minv)+minv;//vol_d/vol_tex_d;
	vec3 rayStop = (eye.Origin + eye.Dir * tfar)*(maxv-minv)+minv;//vol_d/vol_tex_d;

	vec3 dir = rayStop - rayStart; // starting position of the ray

	vec = rayStart;
	float dl = length(dir);
	if(dl == clamp(dl,0.,vol_l)) 
	{
		//int steps = int(round(length(vold * dir) * quality));
		int steps = int(floor(length(vold * dir) * quality));
		vec3 delta_dir = dir/float(steps);
		float aScale =  density/quality;

		float random = fract(sin(gl_FragCoord.x * 12.9898 + gl_FragCoord.y * 78.233) * 43758.5453);
		vec += delta_dir * random;

		//raycast
		for(int i = 0; i < steps / 16.; i++)
		{
			vec4 colorSample0 = textureLod(volume_tex, vec, 4);
			if (colorSample0.a > threshold)
			{
				for (int j = 0; j < 4; j++)
				{
					vec4 colorSample1 = textureLod(volume_tex, vec, 2);
					if (colorSample1.a > threshold)
					{
						for (int k = 0; k < 2; k++)
						{
							vec4 colorSample2 = textureLod(volume_tex, vec, 1);
							if (colorSample2.a > threshold)
							{
								for (int h = 0; h < 2; h++)
								{
									vec4 color = texture(volume_tex, vec) * aScale;
									float oneMinusAlpha = 1. - clamp(col_acc.a, 0., 1.);
									//col_acc.rgb += oneMinusAlpha * vec3(alpha) * alpha;
									col_acc.rgb = mix(col_acc.rgb, color.rgb * color.a, oneMinusAlpha);
									col_acc.a += color.a * oneMinusAlpha;
									col_acc.rgb /= col_acc.a;
									//col_acc.a += color.a * oneMinusAlpha;
									vec += delta_dir;
								}
							}
							else
							{
								vec += delta_dir * 2.0;
							}
						}
					}
					else
					{
						vec += delta_dir * 4.0;
					}
				}
			}
			else
			{
				vec += delta_dir * 16.0;
			}
			//if(col_acc.a >= 1.0) {
			//	break; // terminate if opacity > 1
			//}
		}

		/*for(int i = 0; i < steps; i++)
		{
			float oneMinusAlpha = 1. - clamp(col_acc.a, 0., 1.);
			float alpha = texture(volume_tex, vec).r * aScale;
			col_acc.rgb += oneMinusAlpha * vec3(alpha) * alpha;
			col_acc.a += alpha * oneMinusAlpha;
			if(col_acc.a >= 1.0) {
				break; // terminate if opacity > 1
			}
			vec += delta_dir;
		}*/
	}
	// export the rendered color
	fragColor = col_acc * globalColor;
}
