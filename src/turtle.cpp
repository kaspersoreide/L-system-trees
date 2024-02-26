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
    

void Turtle::buildGPU(GLuint stringBuffer) {
    GLuint treeBuildingShader = loadComputeShader("shaders/compute/simpleInterpret.glsl");
    glUseProgram(treeBuildingShader);
    GLint bufferSize;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, stringBuffer);
    glGetBufferParameteriv(GL_SHADER_STORAGE_BUFFER, GL_BUFFER_SIZE, &bufferSize);
    int cylinderSegments = 6; // how many quad faces drawn per cylinder
    vertexCount = 6 * cylinderSegments * bufferSize / sizeof(GLuint);  //TODO: this is too big, only need a line segment every time turtle moves
    
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, stringBuffer);
    GLuint vertexBuffer, normalBuffer, colorBuffer;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertexBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, vertexCount * 4 * sizeof(float), NULL, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, vertexBuffer);

    glGenBuffers(1, &normalBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, normalBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, vertexCount * 4 * sizeof(float), NULL, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, normalBuffer);

    glGenBuffers(1, &colorBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, colorBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, vertexCount * 4 * sizeof(float), NULL, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, colorBuffer);

    glUniform1ui(0, vertexCount / 2);
    glUniform1f(1, state.width);
    glUniform1f(2, rotationAngle);
    glUniform1i(3, cylinderSegments);
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
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, 0);
}