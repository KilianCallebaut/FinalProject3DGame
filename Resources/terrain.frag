#version 330 core
out vec4 fragColor;
  
in vec2 passTexCoord;
in vec3 passColor;

uniform sampler2D terrainTexture;

void main() {
	//fragColor = vec4(passColor, 1);
    fragColor = texture(terrainTexture, passTexCoord);
}