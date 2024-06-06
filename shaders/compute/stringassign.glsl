#version 430 core

layout (local_size_x = 32) in;

layout (binding = 0) coherent readonly buffer block0
{
    uint input_data[];
};

layout (binding = 1) coherent writeonly buffer block1
{
    //x is index in productions, y is length of output string
    uvec2 output_data[];
};

struct Production {
    uint predecessor;
    float probability;
    uint size;
    uint successor[64]; 
};

layout (binding = 2) coherent readonly buffer block2
{
    uint n_productions;
    Production productions[];
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


void main() {
    uint id = gl_GlobalInvocationID.x;
    uint stringId = 0;
    float randomValue = random(id + input_data[id]);
    float baseValue = 0.0;
    for (int i = 0; i < n_productions; i++) {
        if (productions[i].predecessor == input_data[id]) {
            if (randomValue <= baseValue + productions[i].probability) {
                stringId = i;
                break;
            }
            baseValue += productions[i].probability;
        }
    }
    output_data[id] = uvec2(stringId, productions[stringId].size);
}   