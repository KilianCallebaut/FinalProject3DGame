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
#include <iostream>
#include <cstdio>
#include <chrono>
#include <thread>

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
	shader.uniform1i("texCoordScale", 1);

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
	shader.uniform1f("texCoordScale", 1);
    glBindVertexArray(model.vao);
    glDrawArrays(GL_TRIANGLES, 0, model.vertices.size());
	glBindVertexArray(0);
}

//Calcuate rotation around the xz axis given a direction and rotation b
float calculatexzRotation(Vector3f dir, Vector3f b) {

	Vector3f xa = dir;
	Vector3f xb = normalize(Vector3f(b.x, 0, b.z));

	return -180 * (atan2f(1, 0) - atan2f(xb.z, xb.x)) / Math::PI;
}

//Calcuate the distance between two points.
float calculateDistance(Vector3f a, Vector3f b) {
	return sqrtf((a - b).sqrMagnitude());
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
void drawSurface(ShaderProgram& shader, const Terrain& terrain, Image image, Vector3f position, Vector3f lightPosition, Vector3f lightColor) {
	Matrix4f modelMatrix;
	modelMatrix.translate(position);

	shader.uniformMatrix4f("modelMatrix", modelMatrix);
	shader.uniform3f("lightPosition", lightPosition);
	shader.uniform3f("lightColor", lightColor);
	shader.uniform1f("hasTexCoords", terrain.texCoords.size() > 0);
	shader.uniform1f("ks", terrain.ks);
	shader.uniform3f("ka", terrain.ka);
	shader.uniform3f("kd", terrain.kd);
	shader.uniform1f("texCoordScale", 20);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, image.handle);
	glBindVertexArray(terrain.vao);
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

//Boudingbox class used for collision detection.
class BoundingBox {
public:
	Vector3f vertices[8];
	Vector3f curr_vertices[8];

	//Calcuate the boundingbox of a model
	void calculateBoundingBox(Model m) {

		Vector3f xmax;
		Vector3f ymax;
		Vector3f zmax;
		Vector3f xmin;
		Vector3f ymin;
		Vector3f zmin;
		//Get the min and max values of x, y and z
		for (Vector3f a : m.vertices) {
			if (a.x > xmax.x)
				xmax = a;
			if (a.y > ymax.y)
				ymax = a;
			if (a.z > zmax.z)
				zmax = a;
			if (a.x < xmin.x)
				xmin = a;
			if (a.y < ymin.y)
				ymin = a;
			if (a.z < zmin.z)
				zmin = a;
		}

		vertices[0] = Vector3f(xmax.x, ymax.y, zmax.z);
		vertices[1] = Vector3f(xmax.x, ymin.y, zmax.z);
		vertices[2] = Vector3f(xmin.x, ymax.y, zmax.z);
		vertices[3] = Vector3f(xmin.x, ymin.y, zmax.z);
		vertices[4] = Vector3f(xmax.x, ymax.y, zmin.z);
		vertices[5] = Vector3f(xmax.x, ymin.y, zmin.z);
		vertices[6] = Vector3f(xmin.x, ymax.y, zmin.z);
		vertices[7] = Vector3f(xmin.x, ymin.y, zmin.z);
		for (int i = 0; i < 8; i++) {
			curr_vertices[i] = vertices[i];
		}
	}

	//Scale the bounding box
	void scale(float s) {
		for (int i = 0; i < 8; i++) {
			curr_vertices[i] *= s;
		}
	}

	//Translate the boundingbox
	void translate(Vector3f t) {
		for (int i = 0; i < 8; i++) {
			curr_vertices[i] = curr_vertices[i] + t;
		}
	}

	//Rotate the bounding box
	void rotate(Vector3f r) {
		for (int i = 0; i < 8; i++) {
			curr_vertices[i] = Vector3f(curr_vertices[i].x * cosf(r.y) + curr_vertices[i].z * sinf(r.y), curr_vertices[i].y,
				-curr_vertices[i].x * sinf(r.y) + curr_vertices[i].z * cosf(r.y));
		}
	}

	//Update the bounding box. Inputs are scale, rotation, position.
	void update(float s, Vector3f rot, Vector3f pos) {
		for (int i = 0; i < 8; i++) {
			curr_vertices[i] = vertices[i];

			curr_vertices[i] *= s;

			curr_vertices[i] = Vector3f(curr_vertices[i].x * cosf(rot.y) + curr_vertices[i].z * sinf(rot.y), curr_vertices[i].y,
				-curr_vertices[i].x * sinf(rot.y) + curr_vertices[i].z * cosf(rot.y));

			curr_vertices[i] = curr_vertices[i] + pos;
		}
	}

	//Check if the boudingbox intersects with a given point in space.
	bool intersect(Vector3f point) {
		float xmax = curr_vertices[0].x;
		float ymax = curr_vertices[0].y;
		float zmax = curr_vertices[0].z;
		float xmin = curr_vertices[0].x;
		float ymin = curr_vertices[0].y;
		float zmin = curr_vertices[0].z;
		for (int i = 0; i < 8; i++) {
			if (curr_vertices[i].x > xmax)
				xmax = curr_vertices[i].x;
			if (curr_vertices[i].y > ymax)
				ymax = curr_vertices[i].y;
			if (curr_vertices[i].z > zmax)
				zmax = curr_vertices[i].z;
			if (curr_vertices[i].x < xmin)
				xmin = curr_vertices[i].x;
			if (curr_vertices[i].y < ymin)
				ymin = curr_vertices[i].y;
			if (curr_vertices[i].z < zmin)
				zmin = curr_vertices[i].z;
		}

		return xmax >= point.x && point.x >= xmin && ymax >= point.y && point.y >= ymin && zmax >= point.z && point.z >= zmin;
	}
};

//Main character class
class Character
{
public:
	Model characterModel;
	Model runFrames[24];
	Model idleFrames[59];
	BoundingBox boundingBox;
	Vector3f position;
	Vector3f rotation;
	Vector3f direction = Vector3f(0, 0, 1.0f);

	float scale = 1.0f;
	float speed = 1.0f;
	float rotationspeed = 30.0f * Math::PI / 180.0f;


	//mode: 0=still, 1=running. 2=dead
	int mode = 0;

	int idlecounter;
	int runcounter;

	std::string projectPath;

	//Set the y position of the character
	void setYPosition(float y) {
		position.y = y;
	}

	//Initialize the character
	void initCharacter(std::string ppath, Vector3f  pos, Vector3f rot = Vector3f(0, 0, 0))
	{
		projectPath = ppath;

		position = pos;
		rotation = rot;

		Vector3f ka = Vector3f(1, 1, 1);
		Vector3f kd = Vector3f(1, 1, 1);
		float ks = 96;

		std::string base = projectPath + "Resources\\Models\\Shadowman";
		characterModel = loadModel(base + "\\Shadowman.obj");

		//Create a boudingbox of the character
		boundingBox = BoundingBox();
		boundingBox.calculateBoundingBox(characterModel);
		boundingBox.update(scale, rotation, position);

		characterModel.ka = ka;
		characterModel.kd = kd;
		characterModel.ks = ks;
		//Load all running frames
		for (int i = 0; i < 24; i++) {
			std::string number;
			if (i < 9) {
				number = "0" + std::to_string(i + 1);
			}
			else {
				number = std::to_string(i + 1);
			}
			std::string path = base + "\\RunAnimation\\Shadowmanv_0000" + number + ".obj";

			runFrames[i] = loadModel(path);
			runFrames[i].ka = ka;
			runFrames[i].kd = kd;
			runFrames[i].ks = ks;
		}
		//Load all idle frames
		for (int i = 0; i < 59; i++) {
			std::string number;
			if (i < 9) {
				number = "0" + std::to_string(i + 1);
			}
			else {
				number = std::to_string(i + 1);
			}
			std::string path = base + "\\IdleAnimation\\Shadowmanv_0000" + number + ".obj";

			idleFrames[i] = loadModel(path);
			idleFrames[i].ka = ka;
			idleFrames[i].kd = kd;
			idleFrames[i].ks = ks;
		}

	}

	//Get the next frame of the character
	Model nextFrame()
	{
		boundingBox.update(scale, rotation, position);

		switch (mode) {
		case 0:
			idlecounter += 1;
			if (idlecounter == 59)
				idlecounter = 0;
			return idleFrames[idlecounter];
		case 1:
			runcounter += 1;
			if (runcounter == 24)
				runcounter = 0;
			move();
			return runFrames[runcounter];
		case 2:
			return characterModel;
		}
	}

	//Set the mode of the character to run.
	void run() {
		switch (mode) {
		case 0:
			mode = 1;
			break;
		case 1:
			mode = 0;
			break;
		}
	}

	//Move to a new position based on the direction and speed.
	void move() {
		position += direction * speed;
	}

	//Move back based on the direcition and speed
	void back() {
		position -= direction * speed;
	}

	//Rotate the character
	void rotate(int left) {
		direction = Vector3f(direction.x * cosf((float)left * rotationspeed) + direction.z * sinf((float)left * rotationspeed), direction.y,
			-direction.x * sinf((float)left * rotationspeed) + direction.z * cosf((float)left * rotationspeed));

		rotation += Vector3f(0, left * rotationspeed * 180 / Math::PI, 0);
	}

	//Set the character to dead mode. GAME OVER.
	void die() {
		if (mode != 2) {
			rotation += Vector3f(90, 0, 0);
			mode = 2;
		}
	}
};

//Boss class: its an Andriod.
class Android {
public:
	Model Head;
	Model Body;
	Model Arm;
	float scale = 10.0f;
	BoundingBox boundingBox;

	Vector3f position;
	Vector3f rotation;

	//head
	Vector3f headRotation;
	Vector3f headPosition = Vector3f(scale * 0, scale * 2.8f, scale * 0);

	//arms
	Vector3f larmRotation;
	Vector3f rarmRotation;
	Vector3f larmPosition = Vector3f(scale * 1.4f, scale * 1.5, scale * 0);
	Vector3f rarmPosition = Vector3f(scale * -1.4f, scale * 1.5, scale * 0);

	Vector3f lookTarget;
	bool shooting = false;
	Vector3f shotTarget;
	bool timerS = false;
	std::chrono::time_point<std::chrono::system_clock, std::chrono::system_clock::duration> timer;
	bool reTimerS = false;
	std::chrono::time_point<std::chrono::system_clock, std::chrono::system_clock::duration> rechTimer;

	//projectiles
	float p_speed = 2.0f;

	//General
	Vector3f direction = Vector3f(0, 0, 1.0f);
	std::string projectPath;

	//Initialize the boss.
	void initAndroid(std::string path, Vector3f pos, Vector3f rot = Vector3f(0, 0, 0), Vector3f armRot = Vector3f(0, 0, 0), Vector3f headRot = Vector3f(0, 0, 0)) {
		projectPath = path;

		position = pos;
		larmPosition += pos;
		rarmPosition += pos;
		rotation = rot;
		larmRotation = armRot;
		rarmRotation = armRot;
		headRotation = headRot;

		//Load the multiple components
		Head = loadModel(projectPath + "Resources\\Models\\Android\\android_head.obj");
		Head.ka = Vector3f(0.1, 0, 0);
		Head.kd = Vector3f(0.5, 0, 0);
		Head.ks = 8.0f;
		Body = loadModel(projectPath + "Resources\\Models\\Android\\android_body.obj");
		Body.ka = Vector3f(0.1, 0, 0);
		Body.kd = Vector3f(0.5, 0, 0);
		Body.ks = 8.0f;
		Arm = loadModel(projectPath + "Resources\\Models\\Android\\android_arm.obj");
		Arm.ka = Vector3f(0.1, 0, 0);
		Arm.kd = Vector3f(0.5, 0, 0);
		Arm.ks = 8.0f;

		//Initialize the bouding box.
		boundingBox = BoundingBox();
		boundingBox.calculateBoundingBox(Body);
		boundingBox.update(scale, rotation, position);

	}

	//Update the boss. It aims/looks/shoots at a given target.
	void updateAndroid(Vector3f target) {
		lookTarget = target;
		if (lookTarget != Vector3f(0)) {
			rotateArms();
			rotateHead();
			shootArms();
		}
		if (!shooting) {
			shotTarget = lookTarget;
		}
		if (shooting && shotDone()) {
			resetArms();
		}
	}

	//Set the target of the boss.
	void setTarget(Vector3f target) {
		if (!shooting)
			shotTarget = target;
	}

	//Rotate the arms of the boss.
	void rotateArms() {
		rarmRotation = Vector3f(90, 0, calculatexzRotation(Vector3f(1, 0, 0), normalize(shotTarget - (rarmPosition))));
		larmRotation = Vector3f(90, 0, calculatexzRotation(Vector3f(1, 0, 0), normalize(shotTarget - (larmPosition))));
	}

	//Rotate the head of the boss.
	void rotateHead() {
		headRotation = Vector3f(0, -calculatexzRotation(Vector3f(1, 0, 0), normalize(lookTarget - (position + headPosition))), 0);
	}

	//Shoot the arms of the bot. This has a 3 seconds cooldown.
	void shootArms() {
		if (!timerS && !shooting) {
			timer = std::chrono::system_clock::now() + std::chrono::seconds(3);
			timerS = true;
		}
		if (timerS && std::chrono::system_clock::now() > timer) {
			shooting = true;
			timerS = false;
		}
	}

	//Check if the arms reached the target location.
	bool atTarget(bool left) {
		if (left) {
			return calculateDistance(shotTarget, larmPosition) < 2.0f;
		}
		return calculateDistance(shotTarget, rarmPosition) < 2.0f;
	}

	//Check if the shots have been fired.
	bool shotDone() {
		if (shooting && atTarget(true) && atTarget(false) && !reTimerS) {
			rechTimer = std::chrono::system_clock::now() + std::chrono::seconds(3);
			reTimerS = true;
		}
		if (reTimerS && std::chrono::system_clock::now() > rechTimer) {
			reTimerS = false;
			return true;
		}
		return false;
	}

	//Get the armposition of the boss
	Vector3f armPosition(bool larm) {
		if (shooting) {
			if (larm) {
				larmPosition -= normalize((larmPosition)-shotTarget) * p_speed;
				return larmPosition;
			}
			else {
				rarmPosition -= normalize((rarmPosition)-shotTarget) * p_speed;
				return rarmPosition;
			}
		}
		else {
			if (larm) {
				return larmPosition;
			}
			else {
				return rarmPosition;
			}
		}
	}

	//Reset the arms of the boss back to its body.
	void resetArms() {
		larmPosition = Vector3f(scale * 1.4f, scale * 1.5, scale * 0) + position;
		rarmPosition = Vector3f(scale * -1.4f, scale * 1.5, scale * 0) + position;
		larmRotation = Vector3f(0);
		rarmRotation = Vector3f(0);
		shotTarget = Vector3f(0);
		shooting = false;
	}
};

class Application : KeyListener, MouseMoveListener, MouseClickListener {
public:

	void init() {
		if (emielPC) {
			projectPath = "C:\\users\\Emiel\\Develop\\FinalProject3DGame\\";
		}

		width = window.getWidth();
		height = window.getHeight();
		window.setGlVersion(3, 3, true);

		window.create("Final Project", SCR_WIDTH, SCR_HEIGHT);

		window.addKeyListener(this);
		window.addMouseMoveListener(this);
		window.addMouseClickListener(this);

		//Create shaders
		try {
			//Default shader. used for testing purposes.
			defaultShader.create();
			defaultShader.addShader(VERTEX, projectPath + "Resources\\shader.vert");
			defaultShader.addShader(FRAGMENT, projectPath + "Resources\\shader.frag");
			defaultShader.build();
			//Blinnphong shader
			blinnPhong.create();
			blinnPhong.addShader(VERTEX, projectPath + "Resources\\blinnphong.vert");
			blinnPhong.addShader(FRAGMENT, projectPath + "Resources\\blinnphong.frag");
			blinnPhong.build();
			//Toonshader
			toonShader.create();
			toonShader.addShader(VERTEX, projectPath + "Resources\\toon.vert");
			toonShader.addShader(FRAGMENT, projectPath + "Resources\\toon.frag");
			toonShader.build();
			//Shadow shader for depthmap.
			shadowShader.create();
			shadowShader.addShader(VERTEX, projectPath + "Resources\\shadow.vert");
			shadowShader.build();
		}
		catch (ShaderLoadingException e) {
			std::cerr << e.what() << std::endl;
		}
		projMatrix = orthographic(nn, ff, SCR_WIDTH, SCR_HEIGHT);
		Vector3f initPos = Vector3f(0, 40, -40);
		viewMatrix = lookAtMatrix(initPos, Vector3f(), Vector3f(0, 1.0f, 0));
		viewPosition = initPos;
		viewRotation = initPos;

		//Init shaders.
		// Correspond the OpenGL texture units 0 and 1 with the
		// colorMap and shadowMap uniforms in the shader
		blinnPhong.bind();
		blinnPhong.uniform1i("colorMap", 0);
		blinnPhong.uniform1i("depthMap", 1);

		// Upload the projection matrix once, if it doesn't change
		// during the game we don't need to reupload it
		blinnPhong.uniformMatrix4f("projMatrix", projMatrix);
		
		toonShader.bind();
		toonShader.uniform1i("colorMap", 0);
		toonShader.uniform1i("depthMap", 1);
		toonShader.uniformMatrix4f("projMatrix", projMatrix);

		//Same for the other shaders.
		defaultShader.bind();
		defaultShader.uniformMatrix4f("projMatrix", projMatrix);

		glEnable(GL_DEPTH_TEST);
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

		// Initialize light position and color.
		lightPosition = Vector3f(-200, 200, -200);
		lightColor = Vector3f(0.945, 0.855, 0.643); //Sunlight

		//Init textures
		rockyTerrain = loadImage(projectPath + "Resources\\rockyTerrain.jpg");
		characterTexture = loadImage(projectPath + "Resources\\Textures\\shadowman.jpg");

		//Setup coordsystem (for debugging), shadowFrameBuffer for depth map, and terrain.
		setupCoordSystem();
		setupShadowFrameBuffer();
		terrain = initializeTerrain();

		//Init models
		tmp = loadModel(projectPath + "Resources\\Models\\Housev2.obj");
		tmp.ka = Vector3f(0.3, 0, 0.3);
		tmp.kd = Vector3f(0.3, 0.1, 0.6);
		tmp.ks = 8.0f;

		//Init boss
		android = Android();
		float y = getHeight(200, 200, terrain.heights, terrain.size, terrain.vertexCount);
		android.initAndroid(projectPath, Vector3f(200, y, 200.0f));

		//Init character
		character = Character();
		character.initCharacter(projectPath, Vector3f(0, 0, 0));

		//Test cubes
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
		std::chrono::system_clock::time_point a = std::chrono::system_clock::now();
		std::chrono::system_clock::time_point b = std::chrono::system_clock::now();

        while (!window.shouldClose()) {
			//Framerate management
			a = std::chrono::system_clock::now();
			std::chrono::duration<double, std::milli> work_time = a - b;
			if (work_time.count() < 30.0)
			{
				std::chrono::duration<double, std::milli> delta_ms(30.0 - work_time.count());
				auto delta_ms_duration = std::chrono::duration_cast<std::chrono::milliseconds>(delta_ms);
				std::this_thread::sleep_for(std::chrono::milliseconds(delta_ms_duration.count()));
			}
			b = std::chrono::system_clock::now();
			std::chrono::duration<double, std::milli> sleep_time = b - a;

            // Clear the screen
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			viewMatrix.translate(Vector3f(side, up, forward));
			rotateCamera();
			moveCamera();

			//Get viewposition
			Matrix4f inv = inverse(viewMatrix);
			Vector3f viewPos = Vector3f(inv[12], inv[13], inv[14]);
			

			//Get the next model of the character
			Model charFrame = character.nextFrame();
			detectHit();

			android.updateAndroid(character.position);
			if (android.boundingBox.intersect(character.position)) {
				character.back();
			}

			//Set the height to terrain level (terrain collision detection)
			float y = getHeight(character.position.x, character.position.z, terrain.heights, terrain.size, terrain.vertexCount);
			character.setYPosition(y);

			//shadow
			//Orthographic from light point of view
			Matrix4f lightProjectionMatrix = orthographic(nn, ff, SHADOW_WIDTH, SHADOW_HEIGHT);
			Matrix4f lightViewMatrix = lookAtMatrix(lightPosition, Vector3f(0,0,0), Vector3f(0, 1, 0));
			Matrix4f lightSpaceMatrix = lightProjectionMatrix * lightViewMatrix;
			shadowShader.bind();
			shadowShader.uniformMatrix4f("lightSpaceMatrix", lightSpaceMatrix);
			//Set the viewport of the shadow shader
			glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
				glClear(GL_DEPTH_BUFFER_BIT);
				glCullFace(GL_FRONT);
				//render whole scene for only depthmap
				drawModel(shadowShader, tmp, Vector3f(5, 1, 5), lightPosition, lightColor, Vector3f(0), 2.0f);			
				drawModel(shadowShader, charFrame, character.position, lightPosition, lightColor, character.rotation, character.scale);
				drawModel(shadowShader, android.Body, android.position, lightPosition, lightColor, android.rotation, android.scale);
				drawModel(shadowShader, android.Head, android.headPosition + android.position, lightPosition, lightColor, android.headRotation, android.scale);
				drawModel(shadowShader, android.Arm, android.armPosition(true), lightPosition, lightColor, android.larmRotation, android.scale);
				drawModel(shadowShader, android.Arm, android.armPosition(false), lightPosition, lightColor, android.rarmRotation, android.scale);

				drawSurface(shadowShader, terrain, rockyTerrain, Vector3f(0), lightPosition, lightColor);
				glCullFace(GL_BACK);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			//Set the viewport back to the screen width and height
			glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			//Render objects
			blinnPhong.bind();
			blinnPhong.uniformMatrix4f("viewMatrix", viewMatrix);
			blinnPhong.uniform1f("time", glfwGetTime());
			blinnPhong.uniform3f("viewPos", viewPos);		
			blinnPhong.uniformMatrix4f("lightSpaceMatrix", lightSpaceMatrix);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, depthMap);
			//House render
			drawModel(blinnPhong, tmp, Vector3f(50, getHeight(50,100, terrain.heights, terrain.size, terrain.vertexCount), 100), lightPosition, lightColor, Vector3f(0, 180, 0), 10.0f);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, characterTexture.handle);
			//Character and terrain render.
			drawModel(blinnPhong, charFrame, character.position, lightPosition, lightColor, character.rotation, character.scale);
			drawSurface(blinnPhong, terrain, rockyTerrain, Vector3f(0, 0, 0), lightPosition, lightColor);

			toonShader.bind();
			toonShader.uniformMatrix4f("viewMatrix", viewMatrix);
			toonShader.uniform3f("viewPos", viewPos);
			toonShader.uniformMatrix4f("lightSpaceMatrix", lightSpaceMatrix);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, depthMap);
			 //Boss render
			drawModel(toonShader, android.Body, android.position, lightPosition, lightColor, android.rotation, android.scale);
			drawModel(toonShader, android.Head, android.headPosition + android.position, lightPosition, lightColor, android.headRotation, android.scale);
			drawModel(toonShader, android.Arm, android.armPosition(true), lightPosition, lightColor, android.larmRotation, android.scale);
			drawModel(toonShader, android.Arm, android.armPosition(false), lightPosition, lightColor, android.rarmRotation, android.scale);
			//If we want we can draw a coordsystem.
			if (showCoord) {
				defaultShader.bind();
				defaultShader.uniformMatrix4f("viewMatrix", viewMatrix);
				drawCoordSystem(defaultShader, Vector3f(0, 0, 0), coordVAO);
			}
			//Follow the character.
			viewMatrix = lookAtMatrix(viewPosition + character.position, character.position, Vector3f(0, 1.0f, 0));

			// Processes input and swaps the window buffer
            window.update();
        }

		glDeleteFramebuffers(1, &depthMapFBO);

    }

	void detectHit() {
		if (character.boundingBox.intersect(android.larmPosition) || character.boundingBox.intersect(android.rarmPosition)) {
			character.die();
		}
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
			//forward
			forward += step;
			break;
		case GLFW_KEY_S:
			//backward
			forward -= step;
			break;
		case GLFW_KEY_A:
			//pan left
			side += step;
			break;
		case GLFW_KEY_D:
			//pan right
			side -= step;
			break;
		case GLFW_KEY_R:
			//pan up
			up -= step;
			break;
		case GLFW_KEY_F:
			// pan down
			up += step;
			break;
		case GLFW_KEY_SPACE:
			character.run();
			break;
		case GLFW_KEY_Q:
			angle += cam_rot_sp;
			break;
		case GLFW_KEY_E:
			angle -= cam_rot_sp;
			break;
		case GLFW_KEY_LEFT:
			character.rotate(1);
			break;
		case GLFW_KEY_RIGHT:
			character.rotate(-1);
			break;
		case GLFW_KEY_C:
			showCoord = !showCoord;
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

	void onKeyReleased(int key, int mods)
	{
		switch (key) {
		case GLFW_KEY_W:
			//forward
			forward -= step;
			break;
		case GLFW_KEY_S:
			//backward
			forward += step;
			break;
		case GLFW_KEY_A:
			//pan left
			side -= step;
			break;
		case GLFW_KEY_D:
			//pan right
			side += step;
			break;
		case GLFW_KEY_R:
			//pan up
			up += step;
			break;
		case GLFW_KEY_F:
			//pan down
			up -= step;
			break;
		case GLFW_KEY_SPACE:
			character.run();
			break;
		}
	}

	// If the mouse is moved this function will be called with the x, y screen-coordinates of the mouse
	void onMouseMove(float x, float y) {
		//std::cout << "Mouse at position: " << x << " " << y << std::endl;
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

	void rotateCamera() {
		Vector3f direction = viewPosition;
		float radAngle = angle * Math::PI / 180.f;
		viewPosition = Vector3f(direction.x * cosf((float)radAngle) + direction.z * sinf((float)radAngle), direction.y,
			-direction.x * sinf((float)radAngle) + direction.z * cosf((float)radAngle));
	}

	void moveCamera() {
		viewPosition += Vector3f(0, up, 0);
	}

private:
	Window window;

	// Shader for default rendering and for depth rendering
	ShaderProgram defaultShader;
	ShaderProgram shadowShader;
	ShaderProgram blinnPhong;
	ShaderProgram toonShader;

	// Projection and view matrices for you to fill in and use
	Matrix4f projMatrix;

	Matrix4f viewMatrix;
	Vector3f viewPosition;
	Vector3f viewRotation;
	bool cameraFollow = 1;

	Vector3f lightPosition;
	Vector3f lightColor;

	// Screen/shadow width and height 
	const int SCR_WIDTH = 1024;
	const int SCR_HEIGHT = 1024;
	const unsigned int SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;

	//Coordinate system stuff
	unsigned int coordVAO;
	unsigned int coordVBO;

	bool showCoord = 0;

	//Projection matrix stuff
	float forward = 0;
	float side = 0;
	float up = 0;
	float angle = 0;

	uint width;
	uint height;

	float nn = 0.1f;
	float ff = 1000.0f;
	float step = 0.01f;
	float cam_rot_sp = 2.0;

	//shadow
	GLuint depthMapFBO;
	GLuint depthMap;

	//Terrain
	Terrain terrain;

	//Images
	Image rockyTerrain;
	Image characterTexture;

	Cube cube_01;
	Cube cube_02;

	//Models
	Model tmp;
	Model tmp2;

	Character character;
	Android android;
	bool drawTestModel = 0;

	std::string projectPath = "";
	bool emielPC = 1;
};

int main() {
	Application app;
	app.init();
	app.update();

	return 0;
}