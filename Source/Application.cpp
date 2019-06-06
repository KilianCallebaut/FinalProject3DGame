#include "Model.h"
#include "Image.h"

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


// Rudimentary function for drawing models, feel free to replace or change it with your own logic
// Just make sure you let the shader know whether the model has texture coordinates
void drawModel(ShaderProgram& shader, const Model& model, Vector3f position, Vector3f rotation = Vector3f(0), float scale = 1)
{
    Matrix4f modelMatrix;
    modelMatrix.translate(position);
    modelMatrix.rotate(rotation);
    modelMatrix.scale(scale);
    shader.uniformMatrix4f("modelMatrix", modelMatrix);
    shader.uniform1i("hasTexCoords", model.texCoords.size() > 0);

    glBindVertexArray(model.vao);
    glDrawArrays(GL_TRIANGLES, 0, model.vertices.size());
}

// Produces a look-at matrix from the position of the camera (camera) facing the target position (target)
Matrix4f lookAtMatrix(Vector3f camera, Vector3f target, Vector3f up)
{
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

bool FileExists(const std::string &Filename)
{
	return access(Filename.c_str(), 0) == 0;
}


class Application : KeyListener, MouseMoveListener, MouseClickListener
{
public:
	void init() {
		width = window.getWidth();
		height = window.getHeight();

		window.setGlVersion(3, 3, true);
		window.create("Final Project", width, height);

		window.addKeyListener(this);
		window.addMouseMoveListener(this);
		window.addMouseClickListener(this);

		TCHAR NPath[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, NPath);

		std::cout << NPath << std::endl;



		try {
			defaultShader.create();
			defaultShader.addShader(VERTEX, "Resources//shader.vert");
			defaultShader.addShader(FRAGMENT, "Resources//shader.frag");
			defaultShader.build();

			shadowShader.create();
			shadowShader.addShader(VERTEX, "Resources//shadow.vert");
			shadowShader.build();

			// Any new shaders can be added below in similar fashion
			// ....
		}
		catch (ShaderLoadingException e)
		{
			std::cerr << e.what() << std::endl;
		}

		// Correspond the OpenGL texture units 0 and 1 with the
		// colorMap and shadowMap uniforms in the shader
		defaultShader.bind();
		defaultShader.uniform1i("colorMap", 0);
		defaultShader.uniform1i("shadowMap", 1);

		// Upload the projection matrix once, if it doesn't change
		// during the game we don't need to reupload it
		
		// Perspective
		/*
		float znear = 1.0f;
		float zfar = 100.0f;
		float fov = 110 * (M_PI/180);
		float aspect = (float)width / (float)height;
		
		
		projMatrix[0] = 1.0f / (aspect*tan(fov / 2.0f));
		projMatrix[5] = 1.0f / tan(fov / 2.0f);
		projMatrix[10] = -((zfar + znear) / (zfar - znear));
		projMatrix[11] = -((2 * zfar*znear) / (zfar - znear));
		projMatrix[14] = -1.0f;
		*/

		// Orthographic
		float znear = nn;
		float zfar = ff;
		std::cout << "near" << znear;
		std::cout << "far" << zfar;
		
		float aspect = (float)width / (float)height;

		float top = 1.0f;
		float bottom = -1.0f;
		float right = top * aspect;
		float left = bottom * aspect;

		projMatrix[0] = 2.0f / (right - left);
		projMatrix[3] = -((right + left) / (right - left));
		projMatrix[5] = 2.0f/(top-bottom);
		projMatrix[7] = -((top + bottom) / (top - bottom));
		projMatrix[10] = -2.0f/(zfar-znear);
		projMatrix[11] = -((zfar+znear)/(zfar-znear));
		projMatrix[15] = 1.0f;

		

		defaultShader.uniformMatrix4f("projMatrix", projMatrix);
		
		viewMatrix = lookAtMatrix(Vector3f(0, 2.0f, -3.0f), Vector3f(), Vector3f(0, 1.0f, 0));
		defaultShader.uniformMatrix4f("viewMatrix", viewMatrix);


		glEnable(GL_DEPTH_TEST);
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

		//Init models
		
		tmp = loadModel("Resources//77613_Simple_Character__rigged_//shadowman.obj");

	}

	void update() {
		// This is your game loop
		// Put your real-time logic and rendering in here
		while (!window.shouldClose())
		{
			

			defaultShader.bind();

			// Clear the screen
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


			defaultShader.uniformMatrix4f("projMatrix", projMatrix);

			viewMatrix.translate(Vector3f(side, up, forward));
			defaultShader.uniformMatrix4f("viewMatrix", viewMatrix);

			
			drawModel(defaultShader, tmp, Vector3f());
			/*
			drawModel(defaultShader, tmp, Vector3f(1.0f, 0, 0));
			drawModel(defaultShader, tmp, Vector3f(-1.0f, 0, 0));
			drawModel(defaultShader, tmp, Vector3f(0, 1.0f, 0));
			drawModel(defaultShader, tmp, Vector3f(0, -1.0f, 0));
			drawModel(defaultShader, tmp, Vector3f(0, 0, 1.0f));
			drawModel(defaultShader, tmp, Vector3f(0, 0, -1.0f));
		
			for (float i = 0; i < 100.0f; i++) {
				drawModel(defaultShader, tmp, Vector3f(0, 0, i));
			}
			*/
			// Processes input and swaps the window buffer
			window.update();
		}
	}

	
    // In here you can handle key presses
    // key - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__keys.html
    // mods - Any modifier keys pressed, like shift or control
    void onKeyPressed(int key, int mods)
    {
        std::cout << "Key pressed: " << key << std::endl;
		float step = 0.01f;
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
		case GLFW_KEY_Y:
			nn -= 1.0f;
			break;
		case GLFW_KEY_H:
			nn += 1.0f;
			break;
		case GLFW_KEY_T:
			ff -= 1.0f;
			break;
		case GLFW_KEY_G:
			ff += 1.0f;
			break;
		case GLFW_KEY_R:
			up += step;
			break;
		case GLFW_KEY_F:
			up -= step;
			break;
		}
    }

    // In here you can handle key releases
    // key - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__keys.html
    // mods - Any modifier keys pressed, like shift or control
    void onKeyReleased(int key, int mods)
    {
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
		case GLFW_KEY_R:
			up -= step;
			break;
		case GLFW_KEY_F:
			up += step;
			break;
		}
    }

    // If the mouse is moved this function will be called with the x, y screen-coordinates of the mouse
    void onMouseMove(float x, float y)
    {
        std::cout << "Mouse at position: " << x << " " << y << std::endl;
    }

    // If one of the mouse buttons is pressed this function will be called
    // button - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__buttons.html
    // mods - Any modifier buttons pressed
    void onMouseClicked(int button, int mods)
    {
        std::cout << "Pressed button: " << button << std::endl;
		
    }

    // If one of the mouse buttons is released this function will be called
    // button - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__buttons.html
    // mods - Any modifier buttons pressed
    void onMouseReleased(int button, int mods)
    {

    }

private:
    Window window;

    // Shader for default rendering and for depth rendering
    ShaderProgram defaultShader;
    ShaderProgram shadowShader;

    // Projection and view matrices for you to fill in and use
    Matrix4f projMatrix;
    Matrix4f viewMatrix;

	Model tmp;
	float forward = 0;
	float side = 0;
	float up = 0;
	
	uint width;
	uint height;

	float nn = 1.0f;
	float ff = 10.0f;
};

int main()
{
    Application app;
    app.init();
    app.update();

    return 0;
}
