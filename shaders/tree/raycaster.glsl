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

struct Node {
    uint idx;
    uint parent;
    uvec4 children;     //max 4 children
    vec4 pos;
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
    vec3 x2 = (Model * tree[idx].pos).xyz;
    vec3 x1 = (Model * tree[tree[idx].parent].pos).xyz;
    vec3 cp = cross(point - x1, point - x2);
    float d = length(cross(point - x1, point - x2)) / distance(x2, x1); 
    return d - tree[idx].width;
}

vec3 quadraticSpline(vec3 a, vec3 b, vec3 c, float t) {
    vec3 ab = mix(a, b, t);
    vec3 bc = mix(b, c, t);
    return mix(ab, bc, t);
}
/*
vec3 closestSplinePoint(uint idx, vec3 point) {
    //cubic spline
    float curve = 0.01;
    float distFactor = 0.5;
    vec3 p3 = (Model * tree[idx].pos).xyz;
    vec3 p0 = (Model * tree[tree[idx].parent].pos).xyz;
    vec3 dir = normalize(p3 - p0);
    vec3 parentDir = getParentDir(idx);
    float d = distance(p0, p3);
    vec3 p2 = p3 - d * distFactor * dir;// + curve * randomVec3(p3);
    vec3 p1 = p0 + d * distFactor * parentDir;// - curve * randomVec3(p0);
    float t = closestParameter(p0, p3, point);
    return cubicSpline(p0, p1, p2, p3, t);
}

float splineDist(uint idx, vec3 point) {
    vec3 splinePoint = closestSplinePoint(idx, point);
    return distance(point, splinePoint) 
        - mix(tree[idx].width, tree[tree[idx].parent].width, 0.5);
}

vec3 splineNormal(uint idx, vec3 point) {
    vec3 splinePoint = closestSplinePoint(idx, point);
    return normalize(point - splinePoint);
}

vec3 cylinderNormal(uint idx, vec3 point) {
    //https://gdbooks.gitbooks.io/3dcollisions/content/Chapter1/closest_point_on_line.html
    vec3 a = (Model * tree[idx].pos).xyz;
    vec3 b = (Model * tree[tree[idx].parent].pos).xyz;
    float t = closestParameter(a, b, point);
    vec3 closestPoint = a + (b - a) * t;
    return normalize(point - closestPoint);
}
*/
/*
vec2 replaceMins(vec2 mins, float value) {
    if (value < mins[0]) {
        mins[1] = mins[0];
        mins[0] = value;
    }
    else if (value < mins[1]) {
        mins[1] = value;
    }
    return mins;
}
*/

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
    float t = t_guess;
    vec3 dir = normalize(p3 - p0);
    float d = distance(p0, p3);
    float curve = 0.01;
    float distFactor = 0.5;
    vec3 p2 = p3 - d * distFactor * dir;// + curve * randomVec3(p3);
    vec3 p1 = p0 + d * distFactor * parentDir;// - curve * randomVec3(p0);
    float epsilon = 0.001;
    float w1 = tree[idx].width;
    float w0 = tree[tree[idx].parent].width;

    vec3 splinePoint = cubicSpline(p0, p1, p2, p3, t);
    float dist = distance(rayPos, splinePoint) - mix(w0, w1, t);//distance(rayPos, closestPoint);
    
    while (dist > epsilon && steps < 10) {
        //vec3 a = (Model * tree[treeIdx].pos).xyz;
        
        splinePoint = cubicSpline(p0, p1, p2, p3, t);
        dist = distance(rayPos, splinePoint) - mix(w0, w1, t);
        
        //dist = splineDist(treeIdx, rayPos);
        rayPos += dist * viewDir;
        steps++;
    }
    if (dist > epsilon) {
        discard;
        return;
    }
    vec3 color = vec3(0.5, 0.25, 0.0);
    vec3 lightDir = vec3(1.0, 0.0, 0.0);
    //float brightness = clamp(dot(lightDir, splineNormal(treeIdx, rayPos)), 0.1, 1.0);
    float brightness = 1.0;
    FragColor = vec4(
        brightness * exp(-dist) * color,
        //color,
        1.0
    );
}