#version 430

in vec3 worldPos;
flat in uint treeIdx;
in float t_guess;
flat in vec3 nodeWorldPos;
flat in vec3 parentWorldPos;
flat in vec3 dir;
flat in vec3 parentDir;
flat in float width0;
flat in float width1;

out vec4 FragColor;

uniform layout(location = 0) mat4 Model;
uniform layout(location = 2) vec3 cameraPos;
uniform layout(location = 3) mat4 VP;

vec3 cubicSpline(vec3 p0, vec3 p1, vec3 p2, vec3 p3, float t) {
    //B(t) = (1 - t)^3 P0 + 3 (1 - t)^2 t P1 + 3 (1 - t) t^2 P2 + t^3 P3
    return (1-t)*(1-t)*(1-t) * p0
        + 3*(1-t)*(1-t)*t * p1
        + 3*(1-t)*t*t * p2
        + t*t*t * p3;
}

//////////////////////////////////////////////////////////////////////////////
//procedural wood texture shader code retrieved from https://www.shadertoy.com/view/mdy3R1

float sum2(vec2 v) { return dot(v, vec2(1)); }
float sat(float x) { return clamp(x, 0.0, 1.0); }

float h31(vec3 p3) {
	p3 = fract(p3 * .1031);
	p3 += dot(p3, p3.yzx + 333.3456);
	return fract(sum2(p3.xy) * p3.z);
}

float h21(vec2 p) { return h31(p.xyx); }

float n31(vec3 p) {
	const vec3 s = vec3(7, 157, 113);

	// Thanks Shane - https://www.shadertoy.com/view/lstGRB
	vec3 ip = floor(p);
	p = fract(p);
	p = p * p * (3. - 2. * p);
	vec4 h = vec4(0, s.yz, sum2(s.yz)) + dot(ip, s);
	h = mix(fract(sin(h) * 43758.545), fract(sin(h + s.x) * 43758.545), p.x);
	h.xy = mix(h.xz, h.yw, p.y);
	return mix(h.x, h.y, p.z);
}

// roughness: (0.0, 1.0], default: 0.5
// Returns unsigned noise [0.0, 1.0]
float fbm(vec3 p, int octaves, float roughness) {
	float sum = 0.,
	      amp = 1.,
	      tot = 0.;
	roughness = sat(roughness);
	for (int i = 0; i < octaves; i++) {
		sum += amp * n31(p);
		tot += amp;
		amp *= roughness;
		p *= 2.;
	}
	return sum / tot;
}

vec3 randomPos(float seed) {
	vec4 s = vec4(seed, 0, 1, 2);
	return vec3(h21(s.xy), h21(s.xz), h21(s.xw)) * 1e2 + 1e2;
}

// Returns unsigned noise [0.0, 1.0]
float fbmDistorted(vec3 p) {
	p += (vec3(n31(p + randomPos(0.)), n31(p + randomPos(1.)), n31(p + randomPos(2.))) * 2. - 1.) * 1.12;
	return fbm(p, 8, .5);
}

// vec3: detail(/octaves), dimension(/inverse contrast), lacunarity
// Returns signed noise.
float musgraveFbm(vec3 p, float octaves, float dimension, float lacunarity) {
	float sum = 0.,
	      amp = 1.,
	      m = pow(lacunarity, -dimension);
	for (float i = 0.; i < octaves; i++) {
		float n = n31(p) * 2. - 1.;
		sum += n * amp;
		amp *= m;
		p *= lacunarity;
	}
	return sum;
}

// Wave noise along X axis.
vec3 waveFbmX(vec3 p) {
	float n = p.x * 20.;
	n += .4 * fbm(p * 3., 3, 3.);
	return vec3(sin(n) * .5 + .5, p.yz);
}


// Math
float remap01(float f, float in1, float in2) { return sat((f - in1) / (in2 - in1)); }

// Wood material.
vec3 matWood(vec3 p) {
	float n1 = fbmDistorted(p * vec3(7.8, 1.17, 1.17));
	n1 = mix(n1, 1., .2);
	float n2 = mix(musgraveFbm(vec3(n1 * 4.6), 3., 0., 2.5), n1, .85),
	      dirt = 1. - musgraveFbm(waveFbmX(p * vec3(.01, .15, .15)), 4., .26, 2.4) * .4;
	float grain = 1. - smoothstep(.2, 1., musgraveFbm(p * vec3(500, 6, 1), 2., 2., 2.5)) * .2;
	n2 *= dirt * grain;
    
    // The three vec3 values are the RGB wood colors - Tweak to suit.
	return mix(mix(vec3(.3, .12, .03), 2 * vec3(.25, .11, .04), remap01(n2, .19, .56)), vec3(.52, .32, .19), remap01(n2, .56, 1.));
}
//end of code retrieved from https://www.shadertoy.com/view/mdy3R1
/////////////////////////////////////////////


void main() {
    uint idx = treeIdx;
    vec3 p3 = nodeWorldPos;
    vec3 p0 = parentWorldPos;
    vec3 viewDir = normalize(worldPos - cameraPos);

    //vec3 dir = normalize(p3 - p0);
    float dist = distance(p0, p3);
    float curve = 0.01;
    float distFactor = 0.3;
    vec3 p2 = p3 - dist * distFactor * dir;// + curve * randomVec3(p3);
    vec3 p1 = p0 + dist * distFactor * parentDir;// - curve * randomVec3(p0);
    
    vec3 splinePoint = cubicSpline(p0, p1, p2, p3, t_guess);
    //do raycasting for sphere with center at splinePoint
    /*
    float r = mix(width0, width1, t_guess);
    vec3 l = splinePoint - cameraPos;
    float tc = dot(viewDir, l);
    float d1 = dot(l, l) - tc * tc;
    if (d1 > r * r) {
        discard;
        return;
    }
    */
    vec3 normal = normalize(worldPos - splinePoint);
    float brightness = clamp(dot(normal, vec3(1.0, 0.0, 0.0)), 0.1, 1.0);
    //vec3 color = matWood(worldPos);// * vec3(0.7, 1.0, 0.4);
    vec3 color = vec3(.52, .32, .19);
    FragColor = vec4(brightness * color, 1.0);
}