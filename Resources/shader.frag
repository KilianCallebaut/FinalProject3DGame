#version 330

uniform sampler2D colorMap;
uniform sampler2D shadowMap;

in vec3 passPosition;

out vec4 fragColor;

void main() {
    // Output color value, change from (1, 0, 0) to something else
    fragColor = vec4(0, 1, 0, 1);
}
