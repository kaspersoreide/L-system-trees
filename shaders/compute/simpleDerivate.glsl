#version 430 core

layout (local_size_x = 1) in;

layout (binding = 0) coherent readonly buffer block0
{
    uint stringInLength;
    uint stringIn[];
};

layout (binding = 1) coherent writeonly buffer block1
{
    uint stringOutLength;
    uint stringOut[];
};

struct Production {
    uint predecessor;
    float proability;
    uint size;
    uint successor[64]; 
};

layout (binding = 1) coherent readonly buffer block3
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

uniform layout (location = 0) uint seed;

void main() {
    uint idx = 0;
    for (uint i = 0; i < stringInLength; i++) {
        float randomValue = random(seed);
        float baseValue = 0.0;
        int found = 0;
        for (int j = 0; j < n_productions; j++) {
            if (productions[j].predecessor == stringIn[i]) {
                if (randomValue <= baseValue + productions[j].proability) {
                    for (uint k = 0; k < productions[j].size; k++) {
                        stringOut[idx++] = productions[j].successor[k];
                    }
                    found = 1;
                    break;
                }
                baseValue += productions[i].proability;
            }
        }
        if (!found) {
            stringOut[idx++] = stringIn[i];
        }
        seed = hash(seed);
    }
}