#pragma once
#include <string>
#include <vector>
#include <random>
#include "glutils.h"

using namespace std;

extern GLuint stringAssignShader;
extern GLuint sumShader;
extern GLuint productShader;
extern GLuint bigsumShader;

class Lsystem {
public:
    struct Production {
        // for productions with same in, probabilities must add up to 1
        char in;
        string out;
        float probability;
    };

    struct ShaderProduction {
        uint in;
        float probability;
        uint outputLength;
        uint output[64];
    };

    Lsystem() {};
    ~Lsystem();
    void addRule(char in, string out, float probability);
    void setAxiom(string s);
    void iterate(int n);
    void iterateParallel(int n, uint seed);
    void loadProductionsBuffer();
    void swapBuffers();
    
    vector<Production> rules;
    string product;

    GLuint inputBuffer;
    GLuint outputBuffer;
    GLuint productionsBuffer;
    uint n_nodes = 0;
    uint n_leaves = 0;
    int stringSize;
};