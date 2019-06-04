#pragma once

#include <GDT/OpenGL.h>
#include <GDT/Vector2f.h>
#include <GDT/Vector3f.h>
#include <GDT/Vector4f.h>

#include <vector>
#include <string>

class Model
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

Model loadModel(std::string path);
