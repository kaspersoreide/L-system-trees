#include "lsystem.h"
#include "loadshaders.h"

Lsystem::~Lsystem() {
    glDeleteBuffers(1, &inputBuffer);
    glDeleteBuffers(1, &outputBuffer);
    glDeleteBuffers(1, &productionsBuffer);
}

void Lsystem::addRule(char in, string out, float probability) {
    rules.push_back({in, out, probability});
}

void Lsystem::setAxiom(string s) {
    product = s;
}

void Lsystem::iterate(int n) {
    for (int j = 0; j < n; j++) {
        string newProduct = "";
        for (int i = 0; i < product.length(); ++i) {
            char c = product[i];
            string replacement;
            replacement += c;
            float randomValue = ((float)rand()/(float)RAND_MAX);
            float baseValue = 0.0f;
            for (Production p : rules) {
                if (p.in == c) {
                    if (randomValue < baseValue + p.probability) {
                        replacement = p.out;
                        break;
                    }
                    baseValue += p.probability;
                }
            }
            newProduct += replacement;
        }
        product = newProduct;
    }
    
    
    vector<uint32_t> uintString;
    for (char c : product) {
        if (c == 'L') n_leaves++;
        if (c == 'F') n_nodes++;
        uintString.push_back(c);
    }
    //cout << product << '\n';
    stringSize = uintString.size();
    glCreateBuffers(1, &inputBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, inputBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, stringSize * sizeof(uint), uintString.data(), GL_STATIC_DRAW);
}

void Lsystem::loadProductionsBuffer() {
    glCreateBuffers(1, &productionsBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, productionsBuffer);
    vector<ShaderProduction> productions;
    for (int i = 0; i < rules.size(); i++) {
        Production p = rules[i];
        ShaderProduction sp;
        for (int j = 0; j < p.out.size(); j++) {
            sp.output[j] = uint(p.out[j]);
        }
        sp.in = uint(p.in);
        sp.outputLength = uint(p.out.size());
        sp.probability = p.probability;
        productions.push_back(sp);
    }
    uint totalBufferSize = sizeof(uint) + sizeof(ShaderProduction) * productions.size();
    glBufferData(GL_SHADER_STORAGE_BUFFER, totalBufferSize, NULL, GL_STATIC_DRAW);
    uint n_productions = uint(productions.size());
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(uint), &n_productions);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(uint), sizeof(ShaderProduction) * productions.size(), productions.data());
}

void Lsystem::swapBuffers() {
    GLuint tmp = inputBuffer;
    inputBuffer = outputBuffer;
    outputBuffer = tmp;
}

void Lsystem::iterateParallel(int n, uint seed) {
    
    vector<uint> uintString;
    for (char c : product) uintString.push_back(c);
    stringSize = uintString.size();
    loadProductionsBuffer();
    glDeleteBuffers(1, &inputBuffer);
    glDeleteBuffers(1, &outputBuffer);
    glCreateBuffers(1, &inputBuffer);
    glCreateBuffers(1, &outputBuffer);
    
   
    //copy string into input buffer
    int chunkSize = 2048;
    int bufferSize = ceilf(float(stringSize) / chunkSize) * chunkSize;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, inputBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, stringSize * sizeof(uint), uintString.data(), GL_DYNAMIC_DRAW);

    for (int i = 0; i < n; i++) {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputBuffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize * sizeof(uvec2), nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, inputBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, outputBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, productionsBuffer);
        glUseProgram(stringAssignShader);
        glUniform1ui(0, seed);
        glDispatchCompute(stringSize / 32 + 1, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glFinish();

        //scansum
        swapBuffers();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, inputBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputBuffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize * sizeof(uvec2), NULL, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, outputBuffer);
        glUseProgram(sumShader);
        uint numGroups = bufferSize / chunkSize;
        glDispatchCompute(numGroups, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glFinish();

        if (numGroups > 1) {
            swapBuffers();
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, inputBuffer);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, outputBuffer);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, productionsBuffer);
            glUseProgram(bigsumShader);
            glDispatchCompute(numGroups, 1, 1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            glFinish();
        }
        

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputBuffer);
        uvec2 lastBufferEntry; 
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, (2 * stringSize - 2) * sizeof(uint), 2 * sizeof(uint), &lastBufferEntry);
        int newStringSize = lastBufferEntry.y + rules[lastBufferEntry.x].out.size();
        //genproduct
        swapBuffers();
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputBuffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, newStringSize * sizeof(uint), NULL, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, inputBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, inputBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, outputBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, productionsBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, productionsBuffer);
        glUseProgram(productShader);
        glDispatchCompute(stringSize, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glFinish();


        swapBuffers();
        stringSize = newStringSize;
        bufferSize = ceilf(float(stringSize) / chunkSize) * chunkSize;
        
    }

}