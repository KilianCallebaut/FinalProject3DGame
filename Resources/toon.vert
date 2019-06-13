#version 330

uniform mat4 projMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform mat4 lightSpaceMatrix;

uniform vec3 lightColor;
uniform vec3 lightPosition;
uniform vec3 viewPos;
uniform vec3 kd;
uniform vec3 ka;
uniform float ks;

in vec4 position;
in vec3 normal;
in vec2 texCoord;
in vec3 color;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} vs_out;

out vec3 lightPos;
out vec3 passLightColor;
out vec3 passkd;
out vec3 passka;
out vec3 passViewPos;
out float passks;

void main() {
    vs_out.FragPos = (modelMatrix * position).xyz;
    vs_out.Normal = transpose(inverse(mat3(modelMatrix))) * normal;
    vs_out.TexCoords = texCoord; //resolution up: scale with an int
    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
    gl_Position = projMatrix * viewMatrix * modelMatrix * position;

	passkd = kd;
	passks = ks;
	passka = ka;
	lightPos = lightPosition;
	passViewPos = viewPos;
	passLightColor = lightColor;
}
	
