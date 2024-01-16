#pragma once
#include <string>
#include <vector>
#include <random>
#include "glutils.h"

using namespace std;

class Lsystem {
public:
    struct Production {
        // for productions with same in, probabilities must add up to 1
        char in;
        string out;
        float probability;
    };

    Lsystem() {};
    void addRule(char in, string out, float probability);
    void setAxiom(string s);
    void iterate(int n);
    void iterateParallel(int n);
    void loadProductionsBuffer();
    void swapBuffers();
    
    vector<Production> rules;
    string product;

    GLuint inputBuffer;
    GLuint outputBuffer;
    GLuint productionsBuffer;
};