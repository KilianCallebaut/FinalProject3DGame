#version 330

uniform mat4 projMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform vec3 lightColor;
uniform vec3 lightPosition;
uniform vec3 kd;
uniform vec3 ka;
uniform float ks;

in vec4 position;
in vec3 normal;
in vec2 texCoord;

out vec3 passPosition;
out vec3 passNormal;
out vec2 passTexCoord;
out vec4 passShadowCoord;
out vec3 lightDir;
out vec3 halfwayDir;
out vec3 passLightColor;
out vec3 passkd;
out vec3 passka;
out float passks;

void main() {
    gl_Position = projMatrix * viewMatrix * modelMatrix * position;
    
	// Process stuff to pass to frag shader.
    passPosition = (modelMatrix * position).xyz;
    passNormal = (modelMatrix * vec4(normal, 0)).xyz; // Same as normal, z and w are 0.
    passTexCoord = texCoord;
	passLightColor = lightColor;
	passkd = kd;
	passks = ks;
	passka = ka;

	lightDir   = normalize(lightPosition - position.xyz);
	vec3 viewDir = normalize(lightDir - position.xyz);
	halfwayDir = normalize(lightDir + viewDir);
}
	