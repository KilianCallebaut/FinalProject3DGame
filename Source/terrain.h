#pragma once

#include <GDT/OpenGL.h>
#include <GDT/Vector2f.h>
#include <GDT/Vector3f.h>

#include <vector>

class Terrain {
public:
	std::vector<Vector3f> vertices;
	std::vector<Vector3f> normals;
	std::vector<Vector2f> texCoords;
	std::vector<unsigned int> indices;

	float size;
	int vertexCount;

	GLuint vao;
};

Terrain initializeTerrain();