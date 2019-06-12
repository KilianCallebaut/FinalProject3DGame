#version 330 core
uniform sampler2D terrainTexture;
uniform sampler2D depthMap;


out vec4 fragColor;
  
//in vec2 passTexCoord;
//in vec3 passColor;
//in vec3 passPosition;
//in vec3 passNormal;

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
	float bias = 0.0000002;
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
	//fragColor = vec4(1,0,0,1);

//	vec3 projCoords = fs_in.FragPosLightSpace.xyz / fs_in.FragPosLightSpace.w;
//    projCoords = projCoords * 0.5 + 0.5;
//	fragColor = texture(terrainTexture, fs_in.TexCoords);
//
	float shadow = ShadowCalculation(fs_in.FragPosLightSpace);
	//fragColor = vec4(1-shadow, 1-shadow, 1-shadow, 1);
//	if(shadow == 1.0) {
//		fragColor = vec4(0,0,0,0);
//	} else {
//		fragColor = vec4(1,1,1,1);
//	}
	//fragColor = fs_in.FragPosLightSpace;
   fragColor = texture(terrainTexture, fs_in.TexCoords) * (1-shadow);
	//fragColor = texture(terrainTexture, fs_in.TexCoords) * vec4(vec3(texture(depthMap, fs_in.TexCoords).r),1.0);
   // float depthValue = texture(depthMap, fs_in.TexCoords).r;
    //fragColor = vec4(vec3(depthValue), 1.0);
}