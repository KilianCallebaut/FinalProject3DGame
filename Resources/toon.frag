#version 330
uniform sampler2D colorMap;
uniform sampler2D depthMap;

uniform bool hasTexCoords;

in vec3 passLightColor;
in vec3 lightPos;
in vec3 passkd;
in vec3 passka;
in vec3 passViewPos;
in float passks;

out vec4 finalColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;


float ShadowCalculation(vec4 fragPosLightSpace) {
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(depthMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
	float bias = 0.00000003;
	float shadow = 0;
	vec2 texelSize = 1.0 / textureSize(depthMap, 0);
	for(int x = -1; x<=1; x++) {
		for(int y = -1; y<=1; y++) {
		        float pcfDepth = texture(depthMap, projCoords.xy + vec2(x, y) * texelSize).r; 
				shadow += (currentDepth - bias) > pcfDepth  ? 1.0 : 0.0;
		 }
	}
    return shadow/9.0;
}  


void main() {
    vec3 normal = normalize(fs_in.Normal);	
    vec3 lightDir   = normalize(lightPos - fs_in.FragPos.xyz);

	float shadow = ShadowCalculation(fs_in.FragPosLightSpace);

    vec3 color = vec3(1, 1, 1);
    if (hasTexCoords) {
        color = texture(colorMap, fs_in.TexCoords).rgb;
	}

	//Ambient
	vec4 ambient = vec4(passka, 1);

	//Diffuse
	float intensity = max(dot(lightDir, normal), 0.0);

	if (intensity >= 0.95)
        color = vec4(1.0,1,1,1.0) * color;
    else if (intensity > 0.5)
        color = vec4(0.7,0.7,0.7,1.0) * color;
    else if (intensity > 0.05)
        color = vec4(0.35,0.35,0.35,1.0) * color;
    else
        color = vec4(0.1,0.1,0.1,1.0) * color;
 
   finalColor = vec4(color, 1) * (1-shadow);
}

