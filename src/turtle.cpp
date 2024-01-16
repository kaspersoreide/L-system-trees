#include "turtle.h"
#include "loadshaders.h"

Turtle::Turtle(float initialStep, float _stepDecay, float angle) {
    state = {vec2(0.0f, 0.0f), 1.57079632679489661923f, initialStep};
    rotationAngle = angle;
    stepDecay = _stepDecay;
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
                state.step *= 1.0f - stepDecay;
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
                state.pos += state.step * dir;
                vertices.push_back(state.pos);
                break;
        }
        
    }
}
    

void Turtle::buildGPU(GLuint stringBuffer) {
    GLuint treeBuildingShader = loadComputeShader("shaders/compute/simpleInterpret.glsl");
    glUseProgram(treeBuildingShader);
    GLint bufferSize;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, stringBuffer);
    glGetBufferParameteriv(GL_SHADER_STORAGE_BUFFER, GL_BUFFER_SIZE, &bufferSize);
    vertexCount = 2 * bufferSize / sizeof(GLuint);  //TODO: this is too big, only need a line segment every time turtle moves
 
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, stringBuffer);
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, VBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, vertexCount * 4 * sizeof(float), NULL, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, VBO);

    /*
    uniform layout (location = 0) uint stringLength;
    uniform layout (location = 1) float segmentLength;
    uniform layout (location = 2) float turnAngle;
    */

    glUniform1ui(0, vertexCount / 2);
    glUniform1f(1, state.step);
    glUniform1f(2, rotationAngle);
    glDispatchCompute(1, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    /*
    //print
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, VBO);
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
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

}