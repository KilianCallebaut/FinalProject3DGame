#version 330
uniform sampler2D colorMap;
uniform sampler2D shadowMap;

in vec3 passPosition;
in vec3 passNormal;
in vec3 passTexCoord;

out vec4 fragColor;

void main() {
    vec3 normal = normalize(passNormal);	

    // Output color value, change from (1, 0, 0) to something else
    fragColor = vec4(1, 1, 0, 1);
}
