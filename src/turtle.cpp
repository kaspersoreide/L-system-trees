#include "turtle.h"
#include "loadshaders.h"

Turtle::Turtle(float initialWidth, float _widthDecay, float angle) {
    state = {vec2(0.0f, 0.0f), 1.57079632679489661923f, initialWidth};
    rotationAngle = angle;
    widthDecay = _widthDecay;
}

void Turtle::pushState() {
    stateStack.push(state);
}
void Turtle::popState() {
    state = stateStack.top();
    stateStack.pop();
}

void Turtle::build(string buildString) {
    for (int i = 0; i < buildString.length(); i++) {
        
        switch (buildString[i]) {
            case '[':
                pushState();
                state.width *= 1.0f - widthDecay;
                break;
            case ']':
                popState();
                break;
            case '+':
                state.angle += rotationAngle;
                break;
            case '-':
                state.angle -= rotationAngle;
                break;
            default:
                vec2 dir = {cosf(state.angle), sinf(state.angle)};
                vertices.push_back(state.pos);
                state.pos += state.width * dir;
                vertices.push_back(state.pos);
                break;
        }
        
    }
}
    

void Turtle::buildGPU(GLuint stringBuffer, int cylinderSegments) {
    GLuint treeBuildingShader = loadComputeShader("shaders/compute/simpleInterpret.glsl");
    glUseProgram(treeBuildingShader);

    GLint bufferSize;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, stringBuffer);
    glGetBufferParameteriv(GL_SHADER_STORAGE_BUFFER, GL_BUFFER_SIZE, &bufferSize);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, stringBuffer);

    glGenBuffers(1, &treeBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, treeBuffer);
    //TODO: properly set size of treeBuffer
    glBufferData(GL_SHADER_STORAGE_BUFFER, 8 * bufferSize, NULL, GL_STATIC_DRAW); 
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, treeBuffer);

    glGenBuffers(1, &leafModelsBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, leafModelsBuffer);
    //TODO: properly set size of leafmodelsBuffer
    glBufferData(GL_SHADER_STORAGE_BUFFER, 3 * bufferSize, NULL, GL_STATIC_DRAW); 
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, leafModelsBuffer);

    glUniform1ui(0, bufferSize / sizeof(GLuint)); //string size (number of characters)
    glUniform1f(1, state.width);
    glUniform1f(2, rotationAngle);
    glUniform1i(3, cylinderSegments);
    glUniform1f(4, 5 * state.width);
    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    
    //print
    /*
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, leafModelsBuffer);
    //int outputSize;
    //glGetBufferParameteriv(GL_SHADER_STORAGE_BUFFER, GL_BUFFER_SIZE, &outputSize);
    vector<float> testSum;
    testSum.resize(400);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 400 * sizeof(float), testSum.data());
    
    cout << "buffer data:\n";
    for (int i = 0; i < testSum.size(); i++) {
        cout << ", " << testSum[i];
    }
    */

}