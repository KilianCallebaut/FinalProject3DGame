#version 330

uniform mat4 projMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

in vec4 position;
in vec3 normal;
in vec2 texCoord;

out vec3 passNormal;
out vec2 passTexCoord;

void main()
{
    gl_Position = projMatrix * viewMatrix * modelMatrix * position;
    
    passNormal = (modelMatrix * vec4(normal, 0)).xyz;
    passTexCoord = texCoord;
}
