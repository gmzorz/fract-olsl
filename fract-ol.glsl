#version 330 core

#define ITERATIONS	15
#define POWER		8.0
#define EPSILON		0.005
#define N_EPSILON	0.01
#define MAX_STEPS	100
#define MAX_DIST	30.0
#define PI			3.14159265358979323
#define TWO_PI		6.28318530717958648

uniform vec2		iResolution;
uniform float		iTime;
uniform sampler2D	texture1;

out vec4 color;

float seed; //seed initialized in main

float	rnd()
{
	return (fract(sin(seed++)*43758.5453123));
}

vec2	get_polar_coords(vec3 dir)
{
	vec2	polar;
	polar.x = 0.5 + atan(dir.z, dir.x) / (PI * 2);
	polar.y = 0.5 - asin(-dir.y) / PI;
	return (polar);
}

vec3	opTwist(vec3 p, float k)
{
    float	c = cos(k*p.y);
    float	s = sin(k*p.y);
    mat2 	m = mat2(c,-s,s,c);
    vec3 	q = vec3(m*p.xz,p.y);
    return q;
}

//	https://iquilezles.org/www/articles/mandelbulb/mandelbulb.htm
float	bulb(vec3 p)
{
    vec3 w = p;
    float m = dot(w,w);
	float dz = 1.1;
	for(int i = 0; i < ITERATIONS; i++)
    {
		w = opTwist(-w.xyz, -sin(iTime));
		w = opTwist(-w.zxy, cos(iTime));
		w = opTwist(-w.yzx, -cos(iTime));
		dz = POWER * pow(m,3.5) * dz + 1.0;
        float r = length(w);
        float b = POWER * acos( w.y / r  ) - iTime * 2.;
        float a = POWER * atan( w.x, w.z ) + iTime * 2.;
        w = p + pow(r,POWER) * vec3( sin(b) * sin(a), cos(b), sin(b) * cos(a) );
        m = dot(w,w);
		if( m > 100.0 )
            break;
    }
	//		???????????????????????
    return (0.25 * log(m) * sqrt(m) / dz);
}

float	getDist(vec3 p)
{
	vec3	mandp = -p.yzx;
	mandp.y /= 1.5;
	float	mand = bulb(mandp);
	return (mand);
}

vec3	getNormal(vec3 p)
{
	float	dist = getDist(p);
	vec3	nml;

	nml.x = getDist(p + vec3(dist, 0, 0));
	nml.y = getDist(p + vec3(0, dist, 0));
	nml.z = getDist(p + vec3(0, 0, dist));
	return (normalize(nml - vec3(dist)));
}

bool	is_shadow(vec3 ro, vec3 rd, vec3 l)
{
	float	len = length(l - ro);
	vec3	p = ro;

	//	is facing the other way
	if (dot(rd, l) < 0)
		return (true);
	for (int i = 0; i < 15; i++)
	{
		float t = getDist(p);
		if (t < 0.01)
		{
			//	distance to next surface is shorter than light path
			if (len > length(p))
				return (true);
			break ;
		}
		p += rd * t;
		vec3 one = ro - p;
        if (length(one - normalize(one)) > len)
			break;
	}
	return (false);
}

bool	march(vec3 ro, vec3 rd, out vec3 n, out vec3 hit, out float ao)
{
	hit = ro;
	ao = 0.;
    for (int i = 0; i < MAX_STEPS; i++)
	{
		ao += 1.;
        float t = getDist(hit);
        if (t < EPSILON)
		{
			//	hit surface and calc nml
            n = getNormal(hit);
            for(int i = 0; i < 2; i++)
			{
        		t = 2.0 * EPSILON - getDist(hit);
                hit += n * t;
            }
            return (true);
        }
        hit += rd * t;
		vec3 one = ro - hit;
		//	fixed some bug idk
        if (length(one - normalize(one)) > MAX_DIST)
			break;
    }
	return (false);
}

void coordSystem(vec3 n, out vec3 r, out vec3 u)
{
	if (abs(n.x) > abs(n.y))
		u = vec3(n.z, 0, -n.x) / sqrt(n.x * n.x + n.z * n.z);
	else
		u = vec3(0, -n.z, n.y) / sqrt(n.y * n.y + n.z * n.z);
	r = cross(n, u);
}

vec3	hsphere_sample(float width)
{
	vec3	f = vec3(0);
	vec2	u = vec2(rnd(), rnd());
	float	sintheta = sqrt(1 - u.x * u.y);
	float	phi = PI * 2 * u.y;
	f.x = sintheta * cos(phi) * width;
	f.y = u.x;
	f.z = sintheta * sin(phi) * width;
	return (normalize(f));
}

//	Sample adjustable hemisphere (for eg cone sampling)
vec3 hsphere(vec3 n, float width)
{
	vec3	local = hsphere_sample(width);
	vec3	sampled, r, u;

	coordSystem(n, r, u);
	sampled.x = local.x * u.x + local.y * n.x + local.z * r.x;
	sampled.y = local.x * u.y + local.y * n.y + local.z * r.y;
	sampled.z = local.x * u.z + local.y * n.z + local.z * r.z;
	return (sampled);
}

//	Credit: Martijn steinrucken
vec3 lookAtVec(vec2 uv, vec3 p, vec3 l)
{
    vec3 f = normalize(l-p),
        r = normalize(cross(vec3(0,1,0), f)),
        u = cross(f,r),
        c = f,
        i = c + uv.x*r + uv.y*u,
        d = normalize(i);
    return d;
}

void	main()
{
	vec2	fc = (gl_FragCoord.xy + 3.5) * 0.5;
	vec2	uv = (fc.xy-.5*iResolution.xy)/iResolution.y * ((sin(iTime / 10.) + 5.0) * 0.2);

	float	rotfact = iTime / 10.0;
	vec3	ro = vec3(sin(rotfact), (sin(rotfact) + 1.) * 0.2, cos(rotfact)) * 3.5;
	vec3	rd = lookAtVec(uv, ro, vec3(0, 0, sin(rotfact)));

	//	Ray march
	vec3 	hit,	normal;
	float	dst,	ao;
	bool 	success = march(ro, rd, normal, hit, ao);

	//	Debugging purposes
	vec3 nmlDebug = ((normal + 1.0) * 0.5);

	//	Ambient occlusion
	ao = clamp(1.0 - (ao / 100.), 0.0, 1.0);

	//	Needed for RNG
	seed = iTime * gl_FragCoord.y * gl_FragCoord.x / iResolution.x + gl_FragCoord.y / iResolution.y;
	if (success)
	{
		vec3	dir = hsphere(normal, 1.0);
		vec3	fcol = texture(texture1, get_polar_coords(dir)).zyx;

		//	Sun pos
		vec3	lightPos = vec3(10, 2, 0);
		//	Sample image based lighting
		for (float i = 1; i < 16; i++)
		{
			float	denom = (i / (i + 1.));
			vec3	current;
			float	ndotd;
			dir = hsphere((normal), 1.0);
			ndotd = max(0.0, dot(normal, dir));
			current = ao * ndotd * (texture(texture1, get_polar_coords(dir)).zyx);
			fcol = (fcol * denom + current * (1. - denom));
		}

		//	Gamma
		color.xyz = pow(fcol.xyz, vec3(0.45454545));

		//	Specular highlights (glossy, sparkling)
		vec3	rflOffset = hsphere(reflect(rd, normal), 0.01);
		vec3 	rfl = texture(texture1, get_polar_coords(rflOffset)).zyx;
		rfl *= pow(max(0, dot(normalize(lightPos), rflOffset)), 64) * 1;

		//	Add soft grainy shadow
		dir = hsphere(normalize(lightPos - hit), 0.2);
		if (is_shadow(hit + normal * N_EPSILON, dir, lightPos))
			color.xyz *= ao;
		else
			color.xyz += rfl.yyy*10;
		
		//	Add hard shadow
		dir = hsphere(normalize(lightPos - hit), 0.005);
		if (is_shadow(hit + normal * N_EPSILON, dir, lightPos))
			color.xyz *= 0.5 * ao;
	}
	else
	{
		//	Blur background
		vec3 accum = texture(texture1, get_polar_coords(rd)).zyx;
		for (int i = 1; i <= 20; i++)
		{
			float	denom = (i / (i + 1));
			vec3	customDir = hsphere(rd, 0.005);
			vec3	current = texture(texture1, get_polar_coords(customDir)).zyx;

			accum = (accum * denom + current * (1. - denom));
		}
		//	Gammer
		color.xyz = pow(accum.xyz, vec3(0.454545));
	}
	color.w = 1.0;
}
