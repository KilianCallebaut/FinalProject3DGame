#pragma once

#include <GDT/OpenGL.h>
#include <GDT/Vector2f.h>
#include <GDT/Vector3f.h>

#include <vector>

class Cube
{
public:
	std::vector<Vector3f> vertices;
	std::vector<Vector3f> normals;
	std::vector<Vector2f> texCoords;
	Vector3f ka;
	Vector3f kd;
	float ks;

	GLuint vao;
};

Cube loadCube();