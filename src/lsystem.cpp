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
    /* buffer is laid out like this:
    struct Production {
        uint input;
        float proability;
        uint outputLength;
        uint output[64]; 
    }

    layout (binding = 2) coherent readonly buffer block3
    {
        uint n_productions;
        Production productions[];
    };
    */


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
   
    /*
    //printing
    vector<uint32_t> bufferdata;
    bufferdata.resize(totalBufferSize);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, totalBufferSize, bufferdata.data());
    cout << "productions buffer: \n";
    for (uint32_t c : bufferdata) {
        if (c < 1000) cout << c << ", ";
        else cout << *(float*)&c << ", ";
    } 
    cout << "\n";
    */
}

void Lsystem::swapBuffers() {
    GLuint tmp = inputBuffer;
    inputBuffer = outputBuffer;
    outputBuffer = tmp;
}

void Lsystem::iterateParallel(int n) {
    
    vector<uint> uintString;
    for (char c : product) uintString.push_back(c);
    stringSize = uintString.size();
    loadProductionsBuffer();
    glDeleteBuffers(1, &inputBuffer);
    glDeleteBuffers(1, &outputBuffer);
    glCreateBuffers(1, &inputBuffer);
    glCreateBuffers(1, &outputBuffer);
    
    GLuint stringAssignShader = loadComputeShader("shaders/compute/stringassign.glsl");
    GLuint sumShader = loadComputeShader("shaders/compute/scansum.glsl");
    GLuint productShader = loadComputeShader("shaders/compute/genproduct.glsl");
    GLuint bigsumShader = loadComputeShader("shaders/compute/scansumsum.glsl");
    
    //copy string into input buffer
    int bufferSize = (stringSize / 1024 + 1) * 1024;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, inputBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, stringSize * sizeof(uint), uintString.data(), GL_DYNAMIC_DRAW);

    for (int i = 0; i < n; i++) {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputBuffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize * sizeof(uvec2), nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, inputBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, outputBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, productionsBuffer);
        glUseProgram(stringAssignShader);
        glDispatchCompute(stringSize / 32 + 1, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        /*
        //print
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputBuffer);
        vector<uint> bufferdata;
        bufferdata.resize(2 * stringSize);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 2 * stringSize * sizeof(uint), bufferdata.data());
        cout << "string assign output: \n";
        for (uint c : bufferdata) {
            cout << c << ", ";
        } 
        cout << "\n";
        */

        //scansum
        swapBuffers();
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, inputBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputBuffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize * sizeof(uvec2), NULL, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, outputBuffer);
        glUseProgram(sumShader);
        uint numGroups = stringSize / 1024 + 1;
        glDispatchCompute(numGroups, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        //cout << "number of work groups for prefix sum: " << numGroups << "\n";
        if (numGroups > 1) {
            /*
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputBuffer);
            vector<uvec2> bufferdata;
            int num = 10;
            bufferdata.resize(num);
            glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, (1024 - num / 2) * sizeof(uvec2), num * sizeof(uvec2), bufferdata.data());
            cout << "prefix sum sum input: \n";
            for (uvec2 c : bufferdata) {
                cout << c.x << ", " << c.y << " ... ";
            } 
            cout << "\n"; 
            */

            swapBuffers();
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, inputBuffer);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, outputBuffer);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, productionsBuffer);
            glUseProgram(bigsumShader);
            glDispatchCompute(numGroups, 1, 1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

            //print
            /*
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputBuffer);
            bufferdata.resize(num);
            glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, (1024 - num / 2) * sizeof(uvec2), num * sizeof(uvec2), bufferdata.data());
            cout << "prefix sum sum output: \n";
            for (uvec2 c : bufferdata) {
                cout << c.x << ", " << c.y << " ... ";
            } 
            cout << "\n"; 
            */
        }
        /*
        //print
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputBuffer);
        bufferdata.clear();
        bufferdata.resize(2 * stringSize);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 2 * stringSize * sizeof(uint), bufferdata.data());
        cout << "prefix sum output: \n";
        for (uint c : bufferdata) {
            cout << c << ", ";
        } 
        cout << "\n";
        */

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputBuffer);
        uvec2 lastBufferEntry; 
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, (2 * stringSize - 2) * sizeof(uint), 2 * sizeof(uint), &lastBufferEntry);
        //vector<uvec2> bufferdata(stringSize);
        //glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 2 * stringSize * sizeof(uint32_t), bufferdata.data());
        //cout << "buffer data after scansum:\n";
        //for (uvec2 d : bufferdata) cout << d.x << ", " << d.y << "\n";
        //resize output buffer to (last offset + length of last string)
        int newStringSize = lastBufferEntry.y + rules[lastBufferEntry.x].out.size();
        //cout << "new string size: " << newStringSize << "\n";
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
        
        /*
        //print
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputBuffer);
        bufferdata.clear();
        bufferdata.resize(newStringSize);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, newStringSize * sizeof(uint), bufferdata.data());
        cout << "genproduct output: \n";
        for (uint c : bufferdata) {
            //char ch = char(c);
            //cout << (ch == 'F');
            cout << char(c);// << ",";
        } 
        cout << "\n";
        */

        swapBuffers();
        stringSize = newStringSize;
        bufferSize = (stringSize / 1024 + 1) * 1024;
        //std::cout << "string size after " << i + 1 << " iterations: " << stringSize << "\n";
    }

}