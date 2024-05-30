#version 430

in vec2 uv;
in vec3 normal;
in float height;

out vec4 FragColor;

uniform sampler2D leafTexture;

void main() {
    //vec3 color0 = vec3(1.0, 0.0, 0.0);
    //vec3 color1 = vec3(1.0, 1.0, 0.3);
    vec4 texValue = texture(leafTexture, uv);
    if (texValue.a == 0.0) {
        discard;
        return;
    }
    vec3 lightDir = vec3(1.0, 0.0, 0.0);
    float brightness = clamp(abs(dot(lightDir, normal)), 0.2, 1.);
    //vec3 color = texValue.g * mix(color0, color1, 0.5 * cos(-0.2 * height) + 0.5);
    vec3 color = texValue.rgb;
    FragColor = vec4(brightness * color, 1.0);
}