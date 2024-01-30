#version 430

in float brightness;

out vec4 color;

void main() {
    color = brightness * vec4(
        0.588,
        0.294,
        0.1,
        1.0
    );
}