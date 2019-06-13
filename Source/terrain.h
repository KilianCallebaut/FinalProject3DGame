#pragma once

#include <GDT/OpenGL.h>
#include <GDT/Vector2f.h>
#include <GDT/Vector3f.h>

#include <vector>

class Terrain {
public:
	std::vector<Vector3f> vertices;
	std::vector<Vector3f> normals;
	std::vector<Vector3f> colors;
	std::vector<Vector2f> texCoords;
	std::vector<unsigned int> indices;

	float size;
	int vertexCount = 128;
	int seed;
	float maxHeight;
	int interpolationSteps;
	float roughness;
	std::vector<std::vector<float>> heights;

	GLuint vao;
};

Terrain initializeTerrain();
Vector3f calculateNormal(int x, int z, int seed, int interpolationSteps, float roughness, float maxHeight);
float generateHeight(int x, int z, int seed, int interpolationSteps, float roughness, float maxHeight);
float getSmoothNoise(int x, int z, int seed);
float getNoise(int x, int z, int seed);
float interpolate(float a, float b, float blend);
float getInterpolatedNoise(float x, float z, int seed);
float getHeight(float xPos, float zPos, std::vector<std::vector<float>> heights, float size, float vertexCount);
float barryCentric(Vector3f p1, Vector3f p2, Vector3f p3, Vector2f pos);