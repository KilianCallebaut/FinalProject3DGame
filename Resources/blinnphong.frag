#version 330
uniform sampler2D colorMap;
uniform sampler2D shadowMap;

uniform bool hasTexCoords;

in vec3 passPosition;
in vec3 passNormal;
in vec2 passTexCoord;
in vec4 passShadowCoord;
in vec3 passLightColor;
in vec3 lightPos;
in vec3 passkd;
in vec3 passka;
in float passks;

out vec4 finalColor;

void main() {
    vec3 normal = normalize(passNormal);	
	//lightPos = vec3(sin(time), 1.0, cos(time));
    vec3 lightDir   = normalize(lightPos - passPosition.xyz);
	vec3 viewDir = normalize(lightDir - passPosition.xyz);
	vec3 halfwayDir = normalize(lightDir + viewDir);

		
    vec3 color = vec3(1, 1, 1);
    if (hasTexCoords) {
        color = texture(colorMap, passTexCoord).rgb;
	}

	//Ambient
	vec4 ambient = vec4(passka, 1);

	//Diffuse
	vec4 diffuse = vec4(passkd, 1) * max(dot(lightDir, normal), 0.0) * vec4(passLightColor,1);

	//Specular
	float spec = pow(max(dot(normal, halfwayDir), 0.0), passks);
	vec4 specular = vec4((passLightColor * spec),1);
	
    finalColor = specular + ambient + diffuse;
}

