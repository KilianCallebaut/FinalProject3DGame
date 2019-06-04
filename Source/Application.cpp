#include "Model.h"
#include "Image.h"

#include <GDT/Window.h>
#include <GDT/Input.h>
#include <GDT/Shader.h>
#include <GDT/Matrix4f.h>
#include <GDT/Vector3f.h>
#include <GDT/Math.h>
#include <GDT/OpenGL.h>

#include <vector>
#include <iostream>

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
        window.create("Final Project", 1024, 1024);

        window.addKeyListener(this);
        window.addMouseMoveListener(this);
        window.addMouseClickListener(this);

		// Light init pos.
		lightPosition = Vector3f(1, 1, 1); //position it at  1 1 1
		lightColor = Vector3f(1, 1, 1); //White
			
		setupCoordSystem();

		try {
            defaultShader.create();
            defaultShader.addShader(VERTEX, "C:/users/Emiel/Develop/FinalProject3DGame/Resources/shader.vert");
            defaultShader.addShader(FRAGMENT, "C:/users/Emiel/Develop/FinalProject3DGame/Resources/shader.frag");
            defaultShader.build();

			blinnPhong.create();
			blinnPhong.addShader(VERTEX, "C:/users/Emiel/Develop/FinalProject3DGame/Resources/blinnphong.vert");
			blinnPhong.addShader(FRAGMENT, "C:/users/Emiel/Develop/FinalProject3DGame/Resources/blinnphong.frag");
			blinnPhong.build();

            shadowShader.create();
            shadowShader.addShader(VERTEX, "C:/users/Emiel/Develop/FinalProject3DGame/Resources/shadow.vert");
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
		blinnPhong.bind();
		blinnPhong.uniform1i("colorMap", 0);
		blinnPhong.uniform1i("shadowMap", 1);		
		
		defaultShader.bind();
		defaultShader.uniform1i("colorMap", 0);
		defaultShader.uniform1i("shadowMap", 1);


        // Upload the projection matrix once, if it doesn't change
        // during the game we don't need to reupload it
		blinnPhong.uniformMatrix4f("projMatrix", projMatrix);
		defaultShader.uniformMatrix4f("projMatrix", projMatrix);
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

		//Init models
		tmp = loadModel("C:/users/Emiel/Develop/FinalProject3DGame/dragon.obj");
		tmp.ka = Vector3f(0.1, 0, 0);
		tmp.kd = Vector3f(0.5, 0, 0);
		tmp.ks = 8.0f;
    }

    void update() {
        // This is your game loop
        // Put your real-time logic and rendering in here
        while (!window.shouldClose()) {
            // Clear the screen
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // ...
			blinnPhong.bind();
			blinnPhong.uniformMatrix4f("viewMatrix", viewMatrix);
			drawModel(blinnPhong, tmp, Vector3f(0, 0, 0), lightPosition, lightColor);			
			
			if (showCoord) {
				defaultShader.bind();
				defaultShader.uniformMatrix4f("viewMatrix", viewMatrix);
				drawCoordSystem(defaultShader, Vector3f(0, 0, 0), coordVAO);
			}
			

            // Processes input and swaps the window buffer
            window.update();
        }
    }

	// Setup a coordinate system
	void setupCoordSystem() {
		//Coordsystem lines.
		Vector3f coords[] = {
			Vector3f(0,0,0),
			Vector3f(1,0,0),
			Vector3f(0,1,0),
			Vector3f(0,0,1)
		};
		unsigned int indices[] = {
			0, 1, // x
			0, 2, // y
			0, 3  // z
		};

		unsigned int EBO;
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

		glGenVertexArrays(1, &coordVAO);
		glGenBuffers(1, &coordVBO);

		glBindVertexArray(coordVAO);
		glBindBuffer(GL_ARRAY_BUFFER, coordVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(coords) * sizeof(Vector3f), coords, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}

    // In here you can handle key presses
    // key - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__keys.html
    // mods - Any modifier keys pressed, like shift or control
    void onKeyPressed(int key, int mods) {
		std::cout << "Key pressed: " << key << std::endl;
		switch (key) {
			case 67: // c
				showCoord = !showCoord;
				break;
			default:
				break;
		}
    }

    // In here you can handle key releases
    // key - Integer that corresponds to numbers in https://www.glfw.org/docs/latest/group__keys.html
    // mods - Any modifier keys pressed, like shift or control
    void onKeyReleased(int key, int mods) {

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

    // Projection and view matrices for you to fill in and use
    Matrix4f projMatrix;
    Matrix4f viewMatrix;

	Vector3f lightPosition;
	Vector3f lightColor;

	unsigned int coordVAO;
	unsigned int coordVBO;

	bool showCoord = true;

	Model tmp;
};


int main()
{
    Application app;
    app.init();
    app.update();

    return 0;
}
