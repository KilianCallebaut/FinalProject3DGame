#define _USE_MATH_DEFINES

#include "Terrain.h"
#include <stdlib.h>
#include <ctime>
#include <math.h>


Terrain initializeTerrain() {
	Terrain terrain;

	srand(time(0));
	terrain.seed = rand()%1000000000; //Seed for height generation
	terrain.size = 20; //Length of the terrain
	terrain.vertexCount = 64; //Number of vertices in one direction (x or z)
	terrain.maxHeight = 2.0f; //Maximum height
	terrain.interpolationSteps = 3;
	terrain.roughness = 0.3f;

	int count = terrain.vertexCount * terrain.vertexCount;
	for (int i = 0; i < terrain.vertexCount; i++) {
		for (int j = 0; j < terrain.vertexCount; j++) {
			float x_coord = (float)j / ((float)terrain.vertexCount - 1) * terrain.size;
			float y_coord = generateHeight(j, i, terrain.seed, terrain.interpolationSteps, terrain.roughness, terrain.maxHeight) * terrain.maxHeight;
			float z_coord = (float)i / ((float)terrain.vertexCount - 1) * terrain.size;
			terrain.vertices.push_back(Vector3f(x_coord, y_coord, z_coord));


			Vector3f n = calculateNormal(j, i, terrain.seed, terrain.interpolationSteps, terrain.roughness, terrain.maxHeight);
			terrain.normals.push_back(n);
			//terrain.normals.push_back(Vector3f(0,1,0));

			float r = j % 2;
			float g = 0;
			float b = i % 2;
			terrain.colors.push_back(Vector3f(r, g, b));

			float x_texture = (float)j / ((float)terrain.vertexCount - 1);
			float z_texture = (float)i / ((float)terrain.vertexCount - 1);
			terrain.texCoords.push_back(Vector2f(x_texture, z_texture));
		}
	}

	for (int gz = 0; gz < terrain.vertexCount - 1; gz++) {
		for (int gx = 0; gx < terrain.vertexCount - 1; gx++) {
			int topLeft = gz * terrain.vertexCount + gx;
			int topRight = topLeft + 1;
			int bottomLeft = (gz + 1) * terrain.vertexCount + gx;
			int bottomRight = bottomLeft + 1;

			terrain.indices.push_back(topLeft);
			terrain.indices.push_back(bottomLeft);
			terrain.indices.push_back(topRight);
			terrain.indices.push_back(topRight);
			terrain.indices.push_back(bottomLeft);
			terrain.indices.push_back(bottomRight);
		}
	}
	
	

	glGenVertexArrays(1, &terrain.vao);
	glBindVertexArray(terrain.vao);

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, terrain.vertices.size() * sizeof(Vector3f), terrain.vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	GLuint nbo;
	glGenBuffers(1, &nbo);
	glBindBuffer(GL_ARRAY_BUFFER, nbo);
	glBufferData(GL_ARRAY_BUFFER, terrain.normals.size() * sizeof(Vector3f), terrain.normals.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	GLuint tbo;
	glGenBuffers(1, &tbo);
	glBindBuffer(GL_ARRAY_BUFFER, tbo);
	glBufferData(GL_ARRAY_BUFFER, terrain.texCoords.size() * sizeof(Vector2f), terrain.texCoords.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	GLuint colorbuffer;
	glGenBuffers(1, &colorbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, terrain.colors.size(), terrain.colors.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(3);

	GLuint ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, terrain.indices.size() * sizeof(unsigned int), terrain.indices.data(), GL_STATIC_DRAW);

	return terrain;
}

Vector3f calculateNormal(int x, int z, int seed, int interpolationSteps, float roughness, float maxHeight) {
	float heightLeft = generateHeight(x-1, z, seed, interpolationSteps, roughness, maxHeight);
	float heightRight = generateHeight(x+1, z, seed, interpolationSteps, roughness, maxHeight);
	float heightDown = generateHeight(x, z-1, seed, interpolationSteps, roughness, maxHeight);
	float heightUp = generateHeight(x, z+1, seed, interpolationSteps, roughness, maxHeight);
	return Vector3f(heightLeft-heightRight, 2.0f, heightDown - heightUp).normalize();
}

// Height generator. Loop interpolationSteps times and add it toghether. Each time the results counts less. The more loops the more smooth, but too much might become unrealistic
float generateHeight(int x, int z, int seed, int interpolationSteps, float roughness, float maxHeight) {
	float result = 0.0f;
	float d = (float)pow(2, interpolationSteps)-1;
	for (int i = 0; i < roughness; i++) {
		float f = (float)(pow(2, i) / d);
		float amp = (float)pow(roughness, i) * maxHeight;
		result += getInterpolatedNoise(x,z,seed)*amp;
	}
	return result;
}

//Smoothen the terrain regarding their neighbours and return an avarage value
float getSmoothNoise(int x, int z, int seed) {
	float corners = (getNoise(x-1, z-1, seed) + getNoise(x+1, z-1, seed) + getNoise(x-1, z+1, seed) + getNoise(x+1, z+1, seed)) / 16.0f;
	float sides = (getNoise(x-1, z, seed) + getNoise(x+1, z, seed) + getNoise(x, z-1, seed) + getNoise(x, z+1, seed)) / 8.0f;
	float center = getNoise(x, z, seed) / 4.0f;
	return corners + sides + corners;
}

//Get a random height value
float getNoise(int x, int z, int seed) {
	srand(seed + x * 21421 + z * 8953); //Multiplied by a random big number, because seeds with values close to each other tend to have almost the same result with the random generator.
	return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);;
}

//Get the interpolated noise values for a more smooth terrain.
float getInterpolatedNoise(float x, float z, int seed) {
	int intValueX = (int)x;
	int intValueZ = (int)z;
	float decimalValueX = x - intValueX;
	float decimalValueZ = z - intValueZ;

	float p1 = getSmoothNoise(intValueX, intValueZ, seed);
	float p2 = getSmoothNoise(intValueX + 1, intValueZ, seed);
	float p3 = getSmoothNoise(intValueX, intValueZ + 1, seed);
	float p4 = getSmoothNoise(intValueX + 1, intValueZ + 1, seed);
	float interpolatedValue1 = interpolate(p1, p2, decimalValueX);
	float interpolatedValue2 = interpolate(p3, p4, decimalValueZ);
	return interpolate(interpolatedValue1, interpolatedValue2, decimalValueX);
}

// Cosinus interpolation of the height gives a more smooth and natural feeling of the terrain.
float interpolate(float a, float b, float blend) {
	double theta = blend * M_PI;
	float func = (float)(1.0f - cos(theta)) * 0.5f;
	return a * (1.0f - func) + b * func;
}