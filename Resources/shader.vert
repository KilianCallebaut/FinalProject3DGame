#version 330

uniform mat4 projMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

in vec4 position;

out vec3 passPosition;

void main() {
    gl_Position = projMatrix * viewMatrix * modelMatrix * position;
	passPosition = (modelMatrix * position).xyz;
}
