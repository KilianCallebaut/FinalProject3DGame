#version 330

uniform mat4 projMatrix;
uniform mat4 viewMatrix;

in vec4 position;
in vec3 normal;
in vec2 texCoord;
in vec3 color;

out vec3 passPosition;
out vec3 passNormal;
out vec2 passTexCoord;
out vec3 passColor;

void main() {
    gl_Position = projMatrix * viewMatrix * position;
	passPosition = (position).xyz;
	passNormal = (vec4(normal, 0)).xyz; // Same as normal, z and w are 0.
	passTexCoord = texCoord;
	passColor = color;
}
