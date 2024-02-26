#version 430

in float brightness;
in vec4 FragColor;

out vec4 color;

void main() {
    color = brightness * FragColor;
}