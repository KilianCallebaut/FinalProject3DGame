#version 330

uniform mat4 lightSpaceMatrix;
uniform mat4 modelMatrix;

in vec4 position;
in vec3 normal;

out vec3 fragPos;
out vec3 fragNormal;

void main()
{
    gl_Position = lightSpaceMatrix * modelMatrix * position;

	fragPos = position.xyz;
    fragNormal = normal;
}
