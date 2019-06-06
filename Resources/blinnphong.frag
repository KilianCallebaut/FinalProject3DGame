#version 330

uniform sampler2D colorMap;
uniform sampler2D shadowMap;

uniform bool hasTexCoords;

in vec3 passPosition;
in vec3 passNormal;
in vec2 passTexCoord;
in vec4 passShadowCoord;
in vec3 passLightColor;
in vec3 lightDir;
in vec3 halfwayDir;
in vec3 passkd;
in vec3 passka;
in float passks;

out vec4 finalColor;

void main() {
    vec3 normal = normalize(passNormal);
    
	float shininess = passks;

    vec3 color = vec3(1, 1, 1);
    if (hasTexCoords) {
        color = texture(colorMap, passTexCoord).rgb;
	}

	vec4 ambient = vec4(passka, 1);
	vec4 diffuseColor = vec4(passkd, 1); 
	vec4 diffuse = diffuseColor * max(dot(lightDir, normal), 0.0) * vec4(passLightColor,1);

	float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
	vec4 specular = vec4((passLightColor * spec),1);
	
    // Output color value, change from (1, 0, 0) to something else
    //finalColor = vec4(1, 0, 0, 1) * vec4(lightDir, 1) * vec4(specular, 1);
    finalColor = specular + ambient + diffuse;
    //finalColor = vec4(0.57735, 0.57735, 0.57735, 1);
}


//Components of a vector can be accessed via vec.x 
// where x is the first component of the vector.
// You can use .x, .y, .z and .w to access their first, second, third and fourth component respectively. 
// GLSL also allows you to use rgba for colors or stpq for texture coordinates, accessing the same components.