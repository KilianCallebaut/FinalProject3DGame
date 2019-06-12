#version 330

in vec4 position;
in vec3 normal;
in vec2 texCoord;
in vec3 color;
//
//out vec3 passPosition;
//out vec3 passNormal;
//out vec2 passTexCoord;
//out vec3 passColor;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} vs_out;

uniform mat4 projMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform mat4 lightSpaceMatrix;

void main() {
    vs_out.FragPos = (modelMatrix * position).xyz;
	//vs_out.Normal = (modelMatrix * vec4(normal, 0)).xyz;
    vs_out.Normal = transpose(inverse(mat3(modelMatrix))) * normal;
    vs_out.TexCoords = texCoord;// * 30; //resolution up
    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
    gl_Position = projMatrix * viewMatrix * modelMatrix * position;
//	passPosition = (position).xyz;
//	passNormal = (vec4(normal, 0)).xyz; // Same as normal, z and w are 0.
//	passTexCoord = texCoord;
//	passColor = color;
}

