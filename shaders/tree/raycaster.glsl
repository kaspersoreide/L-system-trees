#version 430

in vec3 worldPos;
flat in uint treeIdx;
in float t_guess;
flat in vec3 nodeWorldPos;
flat in vec3 parentWorldPos;
flat in vec3 dir;
flat in vec3 parentDir;

out vec4 FragColor;

uniform layout(location = 0) mat4 Model;
uniform layout(location = 2) vec3 cameraPos;
uniform layout(location = 3) mat4 VP;

struct Node {
    uint idx;
    uint parent;
    uvec4 children;     //max 4 children
    mat4 T;
    float width;
};

layout (binding = 0) coherent readonly buffer block0
{
    uint lastIdx;   //index of last node in buffer
    Node tree[];   
};

// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
uint hash(uint x) {
	x += (x << 10u);
	x ^= (x >> 6u);
	x += (x << 3u);
	x ^= (x >> 11u);
	x += (x << 15u);
	return x;
}

// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct(uint m) {
	const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
	const uint ieeeOne = 0x3F800000u; // 1.0 in IEEE binary32

	m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
	m |= ieeeOne;                          // Add fractional part to 1.0

	float  f = uintBitsToFloat(m);       // Range [1:2]
	return f - 1.0;                        // Range [0:1]
}

// Pseudo-random value in half-open range [0:1].
float random(uint x) { return floatConstruct(hash(x)); }
vec3 randomVec3(vec3 s) {
    return 2.0 * vec3(
        floatConstruct(hash(floatBitsToUint(s.x))),
        floatConstruct(hash(floatBitsToUint(s.y))),
        floatConstruct(hash(floatBitsToUint(s.z)))
    ) - 1.0;
}

float smin(float a, float b, float k) {
    float h = clamp(0.5 + 0.5*(a-b)/k, 0.0, 1.0);
    return mix(a, b, h) - k*h*(1.0-h);
}

vec3 cubicSpline(vec3 p0, vec3 p1, vec3 p2, vec3 p3, float t) {
    //B(t) = (1 - t)^3 P0 + 3 (1 - t)^2 t P1 + 3 (1 - t) t^2 P2 + t^3 P3
    return (1-t)*(1-t)*(1-t) * p0
        + 3*(1-t)*(1-t)*t * p1
        + 3*(1-t)*t*t * p2
        + t*t*t * p3;
}

float closestParameter(vec3 a, vec3 b, vec3 point) {
    //a and b are endpoints of spline segment, p is point that we want to find closest t value for
    //return t value that is the parameter between 0 and 1, where 0 is a and 1 is b
    //approximated by a line
    vec3 ab = b - a;
    return dot(point - a, ab) / dot(ab, ab);
}

float cylinderDist(uint idx, vec3 point) {
    //approximate by cylinder using distance from point to line
    //https://mathworld.wolfram.com/Point-LineDistance3-Dimensional.html
    vec3 x2 = (Model * tree[idx].T[3]).xyz;
    vec3 x1 = (Model * tree[tree[idx].parent].T[3]).xyz;
    vec3 cp = cross(point - x1, point - x2);
    float d = length(cross(point - x1, point - x2)) / distance(x2, x1); 
    return d - tree[idx].width;
}

vec3 quadraticSpline(vec3 a, vec3 b, vec3 c, float t) {
    vec3 ab = mix(a, b, t);
    vec3 bc = mix(b, c, t);
    return mix(ab, bc, t);
}




///////////////////////////////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////////////////
// Math
float remap01(float f, float in1, float in2) { return sat((f - in1) / (in2 - in1)); }

///////////////////////////////////////////////////////////////////////////////
// Wood material.
vec3 matWood(vec3 p) {
	float n1 = fbmDistorted(p * vec3(7.8, 1.17, 1.17));
	n1 = mix(n1, 1., .2);
	float n2 = mix(musgraveFbm(vec3(n1 * 4.6), 8., 0., 2.5), n1, .85),
	      dirt = 1. - musgraveFbm(waveFbmX(p * vec3(.01, .15, .15)), 15., .26, 2.4) * .4;
	float grain = 1. - smoothstep(.2, 1., musgraveFbm(p * vec3(500, 6, 1), 2., 2., 2.5)) * .2;
	n2 *= dirt * grain;
    
    // The three vec3 values are the RGB wood colors - Tweak to suit.
	return mix(mix(vec3(.03, .012, .003), vec3(.25, .11, .04), remap01(n2, .19, .56)), vec3(.52, .32, .19), remap01(n2, .56, 1.));
}

void main() {
    //vec3 closestPoint = (Model * tree[treeIdx].pos).xyz;
    vec3 viewDir = normalize(worldPos - cameraPos);
    vec3 rayPos = worldPos;
    int steps = 0;
    //check all tree node segments that can intersect with the current one
    //these are the node's child, parent, and parent's child
    /*
    uint indices[10];
    uint numIndices = 1;
    indices[0] = treeIdx;
    for (uint i = 0; i < 3; i++) {
        if (tree[treeIdx].children[i] != 0) {
            indices[numIndices++] = tree[treeIdx].children[i];
        }
        if (tree[treeIdx].parent != 0
            && tree[tree[treeIdx].parent].children[i] != 0
            && tree[tree[treeIdx].parent].children[i] != treeIdx
        ) {
            indices[numIndices++] = tree[tree[treeIdx].parent].children[i];
        }
    }
    if (tree[treeIdx].parent != 0) indices[numIndices++] = tree[treeIdx].parent;
    vec2 minDists = vec2(99999.9); //2 smallest distances
    */
    uint idx = treeIdx;
    vec3 p3 = nodeWorldPos;
    vec3 p0 = parentWorldPos;

    //vec3 dir = normalize(p3 - p0);
    float dist = distance(p0, p3);
    float curve = 0.01;
    float distFactor = 0.3;
    vec3 p2 = p3 - dist * distFactor * dir;// + curve * randomVec3(p3);
    vec3 p1 = p0 + dist * distFactor * parentDir;// - curve * randomVec3(p0);
    
    float w1 = tree[idx].width;
    float w0 = tree[tree[idx].parent].width;
    float epsilon = 0.01 * w1;
    vec3 middlePoint = 0.5 * (p3 + p0);

    float t0 = 0.0;
    float t1 = 1.0;
    vec3 splinePoint;
    float r;
    vec3 l;
    float tc;
    float d0, d1;
    
    while (1 == 1) {
        
        splinePoint = cubicSpline(p0, p1, p2, p3, t0);
        r = mix(w0, w1, t0);
        l = splinePoint - cameraPos;
        tc = dot(viewDir, l);
        d0 = dot(l, l) - tc * tc;

        splinePoint = cubicSpline(p0, p1, p2, p3, t1);
        r = mix(w0, w1, t1);
        l = splinePoint - cameraPos;
        tc = dot(viewDir, l);
        d1 = dot(l, l) - tc * tc;

        float middle = mix(t0, t1, 0.5);
        if (d0 < d1) {
            t1 = middle;
        } else {
            t0 = middle;
        }
    }
    
    if (d1 > r * r) {
        discard;
        return;
    }

    rayPos = cameraPos + (tc - sqrt(r * r - d1)) * viewDir;

    //vec3 color = pow(matWood(rayPos), vec3(.4545));
    vec3 color = vec3(0.6, 0.4, 0.1);
    vec3 lightDir = vec3(1.0, 0.0, 0.0);
    float brightness = clamp(dot(lightDir, normalize(rayPos - splinePoint)), 0.1, 1.0);
    //float brightness = 1.0;
    //vec4 screenPos = VP * vec4(rayPos, 1.0);
    //gl_FragDepth = screenPos.z / screenPos.w;
    FragColor = vec4(
        brightness * color,
        //color,
        1.0
    );
}