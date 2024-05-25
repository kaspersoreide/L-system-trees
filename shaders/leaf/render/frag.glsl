#version 430

in vec2 uv;
in vec3 normal;

out vec4 FragColor;

uniform sampler2D leafTexture;

void main() {
    
    vec4 texValue = texture(leafTexture, uv);
    if (texValue.a == 0.0) {
        discard;
        return;
    }
    vec3 lightDir = vec3(1.0, 0.0, 0.0);
    float brightness = clamp(abs(dot(lightDir, normal)), 0.1, 1.);
    vec3 color = texValue.rgb;
    FragColor = vec4(brightness * color, 1.0);
}