#include "Terrain.h"

Terrain initializeTerrain() {
	Terrain terrain;

	terrain.size = 800; //Length of the terrain
	terrain.vertexCount = 128; //Number of vertices in one direction (x or z)

	int count = terrain.vertexCount * terrain.vertexCount;
	for (int i = 0; i < terrain.vertexCount; i++) {
		for (int j = 0; j < terrain.vertexCount; j++) {
			float x_coord = (float)j / ((float)terrain.vertexCount - 1) * terrain.size;
			float y_coord = 0;
			float z_coord = (float)i / ((float)terrain.vertexCount - 1) * terrain.size;
			terrain.vertices.push_back(Vector3f(x_coord, y_coord, z_coord));

			float x_normal = 0;
			float y_normal = 1;
			float z_normal = 0;
			terrain.normals.push_back(Vector3f(x_normal, y_normal, z_normal));

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

	GLuint ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(terrain.indices), terrain.indices.data(), GL_STATIC_DRAW);

	return terrain;
}