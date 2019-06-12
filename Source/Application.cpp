#include "Model.h"
#include "Cube.h"
#include "Image.h"
#include "Terrain.h"
#include <stb_image.h>

#include <GDT/Window.h>
#include <GDT/Input.h>
#include <GDT/Shader.h>
#include <GDT/Matrix4f.h>
#include <GDT/Vector3f.h>
#include <GDT/Math.h>
#include <GDT/OpenGL.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>
#include <algorithm> 
#include <cmath> 

#define _USE_MATH_DEFINES
#include <math.h>

#ifdef _WIN32
#include <io.h> 
#define access    _access_s
#else
#include <unistd.h>
#endif
#include <ctime>


// Render a cube
void renderCube(ShaderProgram& shader, const Cube& cube, Vector3f position, Vector3f lightPosition, Vector3f lightColor, Vector3f rotation = Vector3f(0), float scale = 1) {
	Matrix4f modelMatrix;
	modelMatrix.translate(position);
	modelMatrix.rotate(rotation);
	modelMatrix.scale(scale);

	shader.uniformMatrix4f("modelMatrix", modelMatrix);
	shader.uniform3f("lightPosition", lightPosition);
	shader.uniform3f("lightColor", lightColor);
	shader.uniform1i("hasTexCoords", cube.texCoords.size() > 0);
	shader.uniform1f("ks", cube.ks);
	shader.uniform3f("ka", cube.ka);
	shader.uniform3f("kd", cube.kd);

	// render Cube
	glBindVertexArray(cube.vao);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

// Rudimentary function for drawing models, feel free to replace or change it with your own logic
// Just make sure you let the shader know whether the model has texture coordinates
void drawModel(ShaderProgram& shader, const Model& model, Vector3f position, Vector3f lightPosition, Vector3f lightColor, Vector3f rotation = Vector3f(0), float scale = 1)
{
    Matrix4f modelMatrix;
    modelMatrix.translate(position);
    modelMatrix.rotate(rotation);
    modelMatrix.scale(scale);
    shader.uniformMatrix4f("modelMatrix", modelMatrix);
	shader.uniform3f("lightPosition", lightPosition);
	shader.uniform3f("lightColor", lightColor);
    shader.uniform1i("hasTexCoords", model.texCoords.size() > 0);
	shader.uniform1f("ks", model.ks);
	shader.uniform3f("ka", model.ka);
	shader.uniform3f("kd", model.kd);

    glBindVertexArray(model.vao);
    glDrawArrays(GL_TRIANGLES, 0, model.vertices.size());
	glBindVertexArray(0);
}

// Produces a look-at matrix from the position of the camera (camera) facing the target position (target)
Matrix4f lookAtMatrix(Vector3f camera, Vector3f target, Vector3f up) {
	Vector3f forward = normalize(target - camera);
	Vector3f right = normalize(cross(forward, up));
	up = cross(right, forward);

	Matrix4f lookAtMatrix;
	lookAtMatrix[0] = right.x; lookAtMatrix[4] = right.y; lookAtMatrix[8] = right.z;
	lookAtMatrix[1] = up.x; lookAtMatrix[5] = up.y; lookAtMatrix[9] = up.z;
	lookAtMatrix[2] = -forward.x; lookAtMatrix[6] = -forward.y; lookAtMatrix[10] = -forward.z;

	Matrix4f translateMatrix;
	translateMatrix[12] = -camera.x;
	translateMatrix[13] = -camera.y;
	translateMatrix[14] = -camera.z;

	return lookAtMatrix * translateMatrix;
}

//Check if a file exists
bool FileExists(const std::string & Filename) {
	return access(Filename.c_str(), 0) == 0;
}

//Draw the surface
void drawSurface(ShaderProgram& shader, const Terrain& terrain, Image image, Vector3f position) {
	Matrix4f modelMatrix;
	modelMatrix.translate(position);

	shader.uniformMatrix4f("modelMatrix", modelMatrix);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, image.handle);
	glBindVertexArray(terrain.vao);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawElements(GL_TRIANGLES, terrain.indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

//function to draw coordinate axes.
void drawCoordSystem(ShaderProgram& shader, Vector3f position, unsigned int vao) {
	Matrix4f modelMatrix;
	modelMatrix.translate(position);

	shader.uniformMatrix4f("modelMatrix", modelMatrix);
	glBindVertexArray(vao);
	glDrawElements(GL_LINES,6, GL_UNSIGNED_INT,0);
	glBindVertexArray(0);
}

class Application : KeyListener, MouseMoveListener, MouseClickListener {
public:

    void init() {
        window.setGlVersion(3, 3, true);
        window.create("Final Project", SCR_WIDTH, SCR_HEIGHT);

        window.addKeyListener(this);
        window.addMouseMoveListener(this);
        window.addMouseClickListener(this);

		// Light init pos.
		lightPosition = Vector3f(-20, 20, -20);
		lightColor = Vector3f(1, 1, 1); //White
		
		//Init surface
		rockyTerrain = loadImage("C:/users/Emiel/Develop/FinalProject3DGame/Resources/rockyTerrain.jpg");
		terrain = initializeTerrain();

		setupCoordSystem();
		setupShadowFrameBuffer();

		try {
            defaultShader.create();
            defaultShader.addShader(VERTEX, "C:/users/Emiel/Develop/FinalProject3DGame/Resources/shader.vert");
            defaultShader.addShader(FRAGMENT, "C:/users/Emiel/Develop/FinalProject3DGame/Resources/shader.frag");
            defaultShader.build();

			blinnPhong.create();
			blinnPhong.addShader(VERTEX, "C:/users/Emiel/Develop/FinalProject3DGame/Resources/blinnphong.vert");
			blinnPhong.addShader(FRAGMENT, "C:/users/Emiel/Develop/FinalProject3DGame/Resources/blinnphong.frag");
			blinnPhong.build();
			
			terrainShader.create();
			terrainShader.addShader(VERTEX, "C:/users/Emiel/Develop/FinalProject3DGame/Resources/terrain.vert");
			terrainShader.addShader(FRAGMENT, "C:/users/Emiel/Develop/FinalProject3DGame/Resources/terrain.frag");
			terrainShader.build();

            shadowShader.create();
            shadowShader.addShader(VERTEX, "C:/users/Emiel/Develop/FinalProject3DGame/Resources/shadow.vert");
            shadowShader.build();
        }
        catch (ShaderLoadingException e) {
            std::cerr << e.what() << std::endl;
        }

		projMatrix = orthographic(nn, ff, SCR_WIDTH, SCR_HEIGHT);
		//viewMatrix = lookAtMatrix(Vector3f(-20,20,-20), Vector3f(), Vector3f(0, 1.0f, 0));
		viewMatrix = lookAtMatrix(Vector3f(0,5,-5), Vector3f(), Vector3f(0, 1.0f, 0));

		//Init shaders.
		defaultShader.bind();
		defaultShader.uniformMatrix4f("projMatrix", projMatrix);

		terrainShader.bind();
		terrainShader.uniform1i("colorMap", 0);
		terrainShader.uniform1i("depthMap", 1);
		terrainShader.uniformMatrix4f("projMatrix", projMatrix);

		// Correspond the OpenGL texture units 0 and 1 with the
        // colorMap and shadowMap uniforms in the shader
		blinnPhong.bind();
		blinnPhong.uniform1i("colorMap", 0);
		blinnPhong.uniform1i("shadowMap", 1);		

        // Upload the projection matrix once, if it doesn't change
        // during the game we don't need to reupload it
		blinnPhong.uniformMatrix4f("projMatrix", projMatrix);
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

		//Init models
		tmp = loadModel("C:/users/Emiel/Develop/FinalProject3DGame/dragon.obj");
		tmp.ka = Vector3f(0.1, 0, 0);
		tmp.kd = Vector3f(0.5, 0, 0);
		tmp.ks = 8.0f;

		cube_01 = loadCube();
		cube_01.ka = Vector3f(0, 0.5, 0.5);
		cube_01.kd = Vector3f(0, 0, 0.1);
		cube_01.ks = 4.0f;
		
		cube_02 = loadCube();
		cube_02.ka = Vector3f(0, 0.5, 0.5);
		cube_02.kd = Vector3f(0, 0, 0.1);
		cube_02.ks = 4.0f;
    }

    void update() {
        // This is your game loop
        // Put your real-time logic and rendering in here
		
        while (!window.shouldClose()) {
            // Clear the screen
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			viewMatrix.translate(Vector3f(side, 0, forward));
			Matrix4f inv = inverse(viewMatrix);
			Vector3f viewPos = Vector3f(inv[12], inv[13], inv[14]);

			//shadow
			//Orthographic from light point of view
			Matrix4f lightProjectionMatrix = orthographic(nn, ff, SHADOW_WIDTH, SHADOW_HEIGHT);
			Matrix4f lightViewMatrix = lookAtMatrix(lightPosition, Vector3f(0,0,0), Vector3f(0, 1, 0));
			Matrix4f lightSpaceMatrix = lightProjectionMatrix * lightViewMatrix;



			shadowShader.bind();
			shadowShader.uniformMatrix4f("lightSpaceMatrix", lightSpaceMatrix);
			glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
				glClear(GL_DEPTH_BUFFER_BIT);
				//render whole scene for only depthmap
				renderCube(shadowShader, cube_01, Vector3f(1, 1, 3), lightPosition, lightColor);
				renderCube(shadowShader, cube_02, Vector3f(3, 1, 1), lightPosition, lightColor);
				drawModel(shadowShader, tmp, Vector3f(5, 1, 5), lightPosition, lightColor, Vector3f(0), 2.0f);
				drawSurface(shadowShader, terrain, rockyTerrain, Vector3f(0));
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			blinnPhong.bind();
			blinnPhong.uniformMatrix4f("viewMatrix", viewMatrix);
			blinnPhong.uniform1f("time", glfwGetTime());
			blinnPhong.uniform3f("viewPos", viewPos);
			drawModel(blinnPhong, tmp, Vector3f(5, 1, 5), lightPosition, lightColor, Vector3f(0), 2.0f);
			renderCube(blinnPhong, cube_02, Vector3f(1, 1, 3), lightPosition, lightColor);
			renderCube(blinnPhong, cube_01, Vector3f(3, 1, 1), lightPosition, lightColor);


			terrainShader.bind();
			terrainShader.uniformMatrix4f("viewMatrix", viewMatrix);
			terrainShader.uniformMatrix4f("lightSpaceMatrix", lightSpaceMatrix);
			terrainShader.uniformMatrix4f("projMatrix", projMatrix);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, depthMap);
			drawSurface(terrainShader, terrain, rockyTerrain, Vector3f(0, 0, 0));
	
			if (showCoord) {
				defaultShader.bind();
				defaultShader.uniformMatrix4f("viewMatrix", viewMatrix);
				drawCoordSystem(defaultShader, Vector3f(0, 0, 0), coordVAO);
			}
			
			// Processes input and swaps the window buffer
            window.update();
        }

		glDeleteFramebuffers(1, &depthMapFBO);

    }

	//Setup the framebuffer that calculates the shadow from the one light point 
	void setupShadowFrameBuffer() {
		//framebuffer object for rendering the depth map
		glGenFramebuffers(1, &depthMapFBO);
		glGenTextures(1, &depthMap);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// attach depth texture as FBO's depth buffer
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// Setup a coordinate system
	void setupCoordSystem() {
		//Coordsystem lines.
		Vector3f coords[] = {
			Vector3f(0,0,0),
			Vector3f(5,0,0),
			Vector3f(0,5,0),
			Vector3f(0,0,5)
		};
		unsigned int indices[] = {
			0, 1, // x
			0, 2, // y
			0, 3  // z
		};

		GLuint EBO;
		GLuint coordVBO;
		glGenBuffers(1, &EBO);

		glGenVertexArrays(1, &coordVAO);
		glGenBuffers(1, &coordVBO);

		glBindVertexArray(coordVAO);
		glBindBuffer(GL_ARRAY_BUFFER, coordVBO);
		glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Vector3f), coords, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}
		
	// Calculate orthographic projection matrix representation of given matrix
	Matrix4f orthographic(float nn, float ff, uint width, uint height) {
		Matrix4f matrix;
		float znear = nn;
		float zfar = ff;
		float aspect = (float)width / (float)height;
		float top = 1.0f;
		float bottom = -1.0f;
		float right = top * aspect;
		float left = bottom * aspect;

		matrix[0] = 2.0f / (right - left);
		matrix[3] = -((right + left) / (right - left));
		matrix[5] = 2.0f / (top - bottom);
		matrix[7] = -((top + bottom) / (top - bottom));
		matrix[10] = -2.0f / (zfar - znear);
		matrix[11] = -((zfar + znear) / (zfar - znear));
		matrix[15] = 1.0f;

		return matrix;
	}

    // In here you can handle key presses
    // key - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__keys.html
    // mods - Any modifier keys pressed, like shift or control
    void onKeyPressed(int key, int mods) {
		std::cout << "Key pressed: " << key << std::endl;
		switch (key) {
		case GLFW_KEY_W:
			forward -= step;
			break;
		case GLFW_KEY_S:
			forward += step;
			break;
		case GLFW_KEY_A:
			side -= step;
			break;
		case GLFW_KEY_D:
			side += step;
			break;
		case GLFW_KEY_R:
			nn -= 1.0f;
			break;
		case GLFW_KEY_F:
			nn += 1.0f;
			break;
		case GLFW_KEY_T:
			ff -= 1.0f;
			break;
		case GLFW_KEY_G:
			ff += 1.0f;
			break;
		case GLFW_KEY_C:
				showCoord = !showCoord;
				break;
		case GLFW_KEY_V:
				drawterrain = !drawterrain;
				break;		
		case GLFW_KEY_B:
				drawTestModel = !drawTestModel;
				break;
		case GLFW_KEY_1:
			//lookAtMatrix();
			break;
		
		}
    }

    // In here you can handle key releases
    // key - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__keys.html
    // mods - Any modifier keys pressed, like shift or control
    void onKeyReleased(int key, int mods) {
		float step = 0.01f;
		switch (key) {
		case GLFW_KEY_W:
			forward += step;
			break;
		case GLFW_KEY_S:
			forward -= step;
			break;
		case GLFW_KEY_A:
			side += step;
			break;
		case GLFW_KEY_D:
			side -= step;
			break;

		}
    }

    // If the mouse is moved this function will be called with the x, y screen-coordinates of the mouse
    void onMouseMove(float x, float y) {
       // std::cout << "Mouse at position: " << x << " " << y << std::endl;
    }

    // If one of the mouse buttons is pressed this function will be called
    // button - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__buttons.html
    // mods - Any modifier buttons pressed
    void onMouseClicked(int button, int mods) {
        std::cout << "Pressed button: " << button << std::endl;
    }

    // If one of the mouse buttons is released this function will be called
    // button - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__buttons.html
    // mods - Any modifier buttons pressed
    void onMouseReleased(int button, int mods) {

    }

private:
    Window window;

    // Shader for default rendering and for depth rendering
    ShaderProgram defaultShader;
    ShaderProgram shadowShader;
    ShaderProgram blinnPhong;
    ShaderProgram terrainShader;

    // Projection and view matrices for you to fill in and use
    Matrix4f projMatrix;
    Matrix4f viewMatrix;

	//Only use this if one static light.
	Vector3f lightPosition;
	Vector3f lightColor;

	// Screen/shadow width and height 
	const int SCR_WIDTH = 1024;
	const int SCR_HEIGHT = 1024;
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

	//Coordinate system stuff
	unsigned int coordVAO;
	bool showCoord = 0;

	//Projection matrix stuff
	float forward = 0;
	float side = 0;
	uint width;
	uint height;
	float nn = 0.1f;
	float ff = 1000.0f;	
	float step = 0.01f;

	//Vertices and texture coordinates for the terrain/water
	int NbVertX = 2, NbVertY = 2;
	//vertices
	std::vector<float> SurfaceVertices3f;
	//normals
	std::vector<float> SurfaceNormals3f;
	//colors
	std::vector<float> SurfaceColors3f;
	//tex coords
	std::vector<float> SurfaceTexCoords2f;
	//triangle indices (three successive entries: n1, n2, n3 represent a triangle, each n* is an index representing a vertex.)
	std::vector<unsigned int> SurfaceTriangles3ui;
	GLuint terrainVAO;

	//shadow
	GLuint depthMapFBO;
	GLuint depthMap;

	//Terrain
	Terrain terrain;
	bool drawterrain = 0;

	//Images
	Image rockyTerrain;

	Cube cube_01;
	Cube cube_02;

	//Models
	Model tmp;
	bool drawTestModel = 0;

	unsigned int cubeVAO = 0;
	unsigned int cubeVBO = 0;
};


int main()
{
    Application app;
    app.init();
    app.update();

    return 0;
}
