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
}

float calculatexzRotation(Vector3f dir, Vector3f b) {

	Vector3f xa = dir;
	Vector3f xb = normalize(Vector3f(b.x, 0, b.z));

	return -180 * (atan2f(1, 0) - atan2f(xb.z, xb.x)) / Math::PI;

}

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
void drawSurface(ShaderProgram& shader, const Terrain& terrain, Image image, Vector3f position) {
	Matrix4f modelMatrix;
	modelMatrix.translate(position);

	shader.uniformMatrix4f("modelMatrix", modelMatrix);
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
	glDrawElements(GL_LINES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}



class BoundingBox {
public:
	/*
	Vector3f ftr;
	Vector3f flr;
	Vector3f ftl;
	Vector3f fll;
	Vector3f btr;
	Vector3f blr;
	Vector3f btl;
	Vector3f bll;*/
	Vector3f vertices[8];

	void calculateBoundingBox(Model m) {

		Vector3f xmax;
		Vector3f ymax;
		Vector3f zmax;
		Vector3f xmin;
		Vector3f ymin;
		Vector3f zmin;
		for (Vector3f a : m.vertices) {
			if (a.x > xmax.x)
				xmax = a;
			if (a.y > ymax.y)
				ymax = a;
			if (a.z > zmax.z)
				zmax = a;
			if (a.x < xmin.x)
				//std::cout << xmin << '\n';
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
	}

	void scale(float s) {
		for (int i = 0; i < 8; i++) {

			vertices[i] *= s;

		}
	}

	void translate(Vector3f t) {
		for (int i = 0; i < 8; i++) {

			vertices[i] += t;

		}
	}

	void rotate(Vector3f r) {
		for (int i = 0; i < 8; i++) {

			vertices[i] = Vector3f(vertices[i].x * cosf(r.y) + vertices[i].z * sinf(r.y), vertices[i].y,
				-vertices[i].x * sinf(r.y) + vertices[i].z * cosf(r.y));;
		}
		/*
		ftr = Vector3f(ftr.x * cosf(r.y) + ftr.z * sinf(r.y), ftr.y,
			-ftr.x * sinf(r.y) + ftr.z * cosf(r.y));
		flr = Vector3f(flr.x * cosf(r.y) + flr.z * sinf(r.y), flr.y,
			-flr.x * sinf(r.y) + flr.z * cosf(r.y));
		ftl = Vector3f(ftl.x * cosf(r.y) + ftl.z * sinf(r.y), ftl.y,
			-ftl.x * sinf(r.y) + ftl.z * cosf(r.y));
		fll = Vector3f(fll.x * cosf(r.y) + fll.z * sinf(r.y), fll.y,
			-fll.x * sinf(r.y) + fll.z * cosf(r.y));
		btr = Vector3f(btr.x * cosf(r.y) + btr.z * sinf(r.y), btr.y,
			-btr.x * sinf(r.y) + btr.z * cosf(r.y));
		blr = Vector3f(blr.x * cosf(r.y) + blr.z * sinf(r.y), blr.y,
			-blr.x * sinf(r.y) + blr.z * cosf(r.y));		
		btl = Vector3f(btl.x * cosf(r.y) + btl.z * sinf(r.y), btl.y,
			-btl.x * sinf(r.y) + btl.z * cosf(r.y));
		bll = Vector3f(bll.x * cosf(r.y) + bll.z * sinf(r.y), bll.y,
			-bll.x * sinf(r.y) + bll.z * cosf(r.y));
			*/
	}

	bool intersect(Vector3f point) {
		float xmax = 0;
		float ymax = 0;
		float zmax = 0;
		float xmin = 0;
		float ymin = 0;
		float zmin = 0;
		for (int i = 0; i < 8; i++) {
			if (vertices[i].x > xmax)
				xmax = vertices[i].x;
			if (vertices[i].y > ymax)
				ymax = vertices[i].y;
			if (vertices[i].z > zmax)
				zmax = vertices[i].z;
			if (vertices[i].x < xmin)
				xmin = vertices[i].x;
			if (vertices[i].y < ymin)
				ymin = vertices[i].y;
			if (vertices[i].z < zmin)
				zmin = vertices[i].z;
		}
		std::cout << point << '\n';
		std::cout << xmax << '\n';
		std::cout << xmin << '\n';
		std::cout << (xmax >= point.x && point.x >= xmin) << '\n';

		return xmax >= point.x && point.x >= xmin && ymax >= point.y && point.y >= ymin && zmax >= point.z && point.z >= zmin;
	}
};

class Character
{
public:
	Model characterModel;
	Model runFrames[24];
	Model idleFrames[59];
	Vector3f position;
	Vector3f rotation;
	Vector3f direction = Vector3f(0, 0, 1.0f);

	float scale = 1.0f;
	float speed = 1.0f;
	float rotationspeed = 30.0f * Math::PI / 180.0f;


	//mode: 0=still, 1=running, 2=dead
	int mode = 0;
	

	int idlecounter;
	int runcounter;

	std::string projectPath;

	Model nextFrame()
	{
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
			position += direction * speed;
			return runFrames[runcounter];
		case 2:
			return characterModel;
		}
	}

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

	void rotate(int left) {

		direction = Vector3f(direction.x * cosf((float)left * rotationspeed) + direction.z * sinf((float)left * rotationspeed), direction.y,
			-direction.x * sinf((float)left * rotationspeed) + direction.z * cosf((float)left * rotationspeed));

		rotation += Vector3f(0, left * rotationspeed * 180 / Math::PI, 0);
	}

	void initCharacter(std::string ppath, Vector3f  pos, Vector3f rot = Vector3f(0, 0, 0))
	{
		projectPath = ppath;

		position = pos;
		rotation = rot;

		std::string base = projectPath + "Resources\\Models\\Shadowman";
		characterModel = loadModel(base + "\\Shadowman.obj");
		characterModel.ka = Vector3f(0.1, 0, 0);
		characterModel.kd = Vector3f(0.5, 0, 0);
		characterModel.ks = 8.0f;
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
			runFrames[i].ka = Vector3f(0.1, 0, 0);
			runFrames[i].kd = Vector3f(0.5, 0, 0);
			runFrames[i].ks = 8.0f;
		}

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
			idleFrames[i].ka = Vector3f(0.1, 0, 0);
			idleFrames[i].kd = Vector3f(0.5, 0, 0);
			idleFrames[i].ks = 8.0f;
		}

	}

	void die() {
		if (mode != 2) {
			rotation += Vector3f(90, 0, 0);
			mode = 2;
		}
	}
};

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
	Vector3f headPosition = Vector3f(scale * 0, scale*2.8f, scale * 0);

	//arms
	Vector3f larmRotation;
	Vector3f rarmRotation;
	Vector3f larmPosition = Vector3f(scale*1.4f, scale*1.5, scale * 0);
	Vector3f rarmPosition = Vector3f(scale*-1.4f, scale*1.5, scale * 0);

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
	

	void initAndroid(std::string path, Vector3f pos, Vector3f rot = Vector3f(0, 0, 0), Vector3f armRot = Vector3f(0, 0, 0), Vector3f headRot = Vector3f(0, 0, 0)) {
		projectPath = path;

		position = pos;
		larmPosition += pos;
		rarmPosition += pos;
		rotation = rot;
		larmRotation = armRot;
		rarmRotation = armRot;
		headRotation = headRot;

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

		boundingBox = BoundingBox();
		boundingBox.calculateBoundingBox(Body);
		
		boundingBox.translate(pos);
		boundingBox.rotate(rotation);
		boundingBox.scale(scale);

	}

	void updateAndroid() {
		if (shotTarget != Vector3f(0)) {
			rotateArms();
			rotateHead();
			shootArms();
		}

		if (shooting && shotDone()) {
			resetArms();
		}
	}

	void setTarget(Vector3f target) {
		if (!shooting)
			shotTarget = target;
	}

	void rotateArms() {
		
		rarmRotation = Vector3f(90, 0, calculatexzRotation(Vector3f(1, 0, 0), normalize(shotTarget - (rarmPosition))));
		larmRotation = Vector3f(90, 0, calculatexzRotation(Vector3f(1, 0, 0), normalize(shotTarget - (larmPosition))));

		//std::cout << armRotation <<'\n';
		//armRotation += Vector3f(0.5f, 0, 0);
		//Vector3f(direction.x*cosf() + direction.z*sinf(), direction.y,
		//		-direction.x*sinf() + direction.z*cosf());
		//direction = Vector3f(direction.x*cosf((float)left*rotationspeed) + direction.z*sinf((float)left*rotationspeed), direction.y,
		//	-direction.x*sinf((float)left*rotationspeed) + direction.z*cosf((float)left*rotationspeed));
	}

	void rotateHead() {

		headRotation = Vector3f(0, -calculatexzRotation(Vector3f(1, 0, 0), normalize(shotTarget - (position + headPosition))), 0);
		//Vector3f(direction.x*cosf() + direction.z*sinf(), direction.y,
		//		-direction.x*sinf() + direction.z*cosf());
		//direction = Vector3f(direction.x*cosf((float)left*rotationspeed) + direction.z*sinf((float)left*rotationspeed), direction.y,
		//	-direction.x*sinf((float)left*rotationspeed) + direction.z*cosf((float)left*rotationspeed));
	}

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

	bool atTarget(bool left) {
		if (left) {
			return calculateDistance(shotTarget, larmPosition) < 2.0f;
		} 
		return calculateDistance(shotTarget, rarmPosition) < 2.0f;
	}

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

	Vector3f armPosition(bool larm) {
		if (shooting) {
			if (larm) {
				larmPosition -= normalize((larmPosition)-shotTarget)*p_speed;
				return larmPosition;
			}
			else {
				rarmPosition -= normalize((rarmPosition)-shotTarget)*p_speed;
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

	void resetArms() {
		larmPosition = Vector3f(scale*1.4f, scale*1.5, scale * 0) + position;
		rarmPosition = Vector3f(scale*-1.4f, scale*1.5, scale * 0) + position;
		larmRotation = Vector3f(0);
		rarmRotation = Vector3f(0);
		shotTarget = Vector3f(0);
		shooting = false;
	}
};

class Application : KeyListener, MouseMoveListener, MouseClickListener {
public:
	void setProjection() {
		// Orthographic
		float znear = nn;
		float zfar = ff;
		std::cout << "near" << znear;
		std::cout << "far" << zfar;

		float aspect = (float)width / (float)height;


		float top = 1.0f;
		float bottom = -top;
		float right = top * aspect;
		float left = -right;

		projMatrix[0] = 2.0f / (right - left);
		projMatrix[3] = -((right + left) / (right - left));
		projMatrix[5] = 2.0f / (top - bottom);
		projMatrix[7] = -((top + bottom) / (top - bottom));
		projMatrix[10] = -2.0f / (zfar - znear);
		projMatrix[11] = -((zfar + znear) / (zfar - znear));
		projMatrix[15] = 1.0f;
	}

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
			defaultShader.create();
			defaultShader.addShader(VERTEX, projectPath + "Resources\\shader.vert");
			defaultShader.addShader(FRAGMENT, projectPath + "Resources\\shader.frag");
			defaultShader.build();

			blinnPhong.create();
			blinnPhong.addShader(VERTEX, projectPath + "Resources\\blinnphong.vert");
			blinnPhong.addShader(FRAGMENT, projectPath + "Resources\\blinnphong.frag");
			blinnPhong.build();

			terrainShader.create();
			terrainShader.addShader(VERTEX, projectPath + "Resources\\terrain.vert");
			terrainShader.addShader(FRAGMENT, projectPath + "Resources\\terrain.frag");
			terrainShader.build();

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
		blinnPhong.uniform1i("shadowMap", 1);

		// Upload the projection matrix once, if it doesn't change
		// during the game we don't need to reupload it
		blinnPhong.uniformMatrix4f("projMatrix", projMatrix);

		//Same for the other shaders.
		defaultShader.bind();
		defaultShader.uniformMatrix4f("projMatrix", projMatrix);

		terrainShader.bind();
		terrainShader.uniform1i("colorMap", 0);
		terrainShader.uniform1i("depthMap", 1);
		terrainShader.uniformMatrix4f("projMatrix", projMatrix);

		glEnable(GL_DEPTH_TEST);
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

		// Initialize light position and color.
		lightPosition = Vector3f(-200, 200, -200);
		lightColor = Vector3f(1, 1, 1); //White

		//Init surface
		rockyTerrain = loadImage(projectPath + "Resources\\rockyTerrain.jpg");
		terrain = initializeTerrain();

		//Setup coordsystem (for debugging) and shadowFrameBuffer for depth map
		setupCoordSystem();
		setupShadowFrameBuffer();

		//Init models
		tmp = loadModel(projectPath + "Resources\\Models\\Housev2.obj");
		tmp.ka = Vector3f(0.1, 0.1, 0.1);
		tmp.kd = Vector3f(0, 0.1, 0);
		tmp.ks = 8.0f;

		//Init boss
		android = Android();
		android.initAndroid(projectPath, Vector3f(0, 0, 5.0f));

		//Init character
		character = Character();
		character.initCharacter(projectPath, Vector3f(0, 0, 0));

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
			viewMatrix.translate(Vector3f(side, 0, forward));
			rotateCamera();

			//Get viewposition
			Matrix4f inv = inverse(viewMatrix);
			Vector3f viewPos = Vector3f(inv[12], inv[13], inv[14]);

			//shadow
			//Orthographic from light point of view
			Matrix4f lightProjectionMatrix = orthographic(nn, ff, SHADOW_WIDTH, SHADOW_HEIGHT);
			Matrix4f lightViewMatrix = lookAtMatrix(lightPosition, Vector3f(0, 0, 0), Vector3f(0, 1, 0));
			Matrix4f lightSpaceMatrix = lightProjectionMatrix * lightViewMatrix;

			//Get the next model of the character
			Model charFrame = character.nextFrame();
			detectHit();
			android.setTarget(character.position);
			android.updateAndroid();
			std::cout << "Intersects: "<< android.boundingBox.intersect(character.position);

			//Set the height to terrain level
			//float y = generateHeight(character.position.z, character.position.z, terrain.heights, terrain.size, terrain.vertexCount);
			//std::cout << y << std::endl;

			shadowShader.bind();
			shadowShader.uniformMatrix4f("lightSpaceMatrix", lightSpaceMatrix);
			glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
			glClear(GL_DEPTH_BUFFER_BIT);
			//render whole scene for only depthmap
			renderCube(shadowShader, cube_01, Vector3f(5, 1, 5), lightPosition, lightColor);
			renderCube(shadowShader, cube_02, Vector3f(3, 1, 1), lightPosition, lightColor);
			//drawModel(shadowShader, tmp, Vector3f(5, 1, 5), lightPosition, lightColor, Vector3f(0), 2.0f);

			drawModel(shadowShader, charFrame, character.position, lightPosition, lightColor, character.rotation, character.scale);
			drawModel(shadowShader, android.Body, android.position, lightPosition, lightColor, android.rotation, android.scale);
			drawModel(shadowShader, android.Head, android.headPosition + android.position, lightPosition, lightColor, android.headRotation, android.scale);
			drawModel(shadowShader, android.Arm, android.armPosition(true), lightPosition, lightColor, android.larmRotation, android.scale);
			drawModel(shadowShader, android.Arm, android.armPosition(false), lightPosition, lightColor, android.rarmRotation, android.scale);

			drawSurface(shadowShader, terrain, rockyTerrain, Vector3f(0));
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			blinnPhong.bind();
			blinnPhong.uniformMatrix4f("viewMatrix", viewMatrix);

			blinnPhong.uniform1f("time", glfwGetTime());
			blinnPhong.uniform3f("viewPos", viewPos);
			//drawModel(blinnPhong, tmp, Vector3f(5, 1, 5), lightPosition, lightColor, Vector3f(0), 2.0f);
			//renderCube(blinnPhong, cube_02, Vector3f(1, 1, 3), lightPosition, lightColor);
			//renderCube(blinnPhong, cube_01, Vector3f(3, 1, 1), lightPosition, lightColor);
			drawModel(blinnPhong, charFrame, character.position, lightPosition, lightColor, character.rotation, character.scale);
			drawModel(blinnPhong, android.Body, android.position, lightPosition, lightColor, android.rotation, android.scale);
			drawModel(blinnPhong, android.Head, android.headPosition + android.position, lightPosition, lightColor, android.headRotation, android.scale);
			drawModel(blinnPhong, android.Arm, android.armPosition(true) , lightPosition, lightColor, android.larmRotation, android.scale);
			drawModel(blinnPhong, android.Arm, android.armPosition(false) , lightPosition, lightColor, android.rarmRotation, android.scale);
			

			terrainShader.bind();
			terrainShader.uniformMatrix4f("viewMatrix", viewMatrix);
			terrainShader.uniformMatrix4f("lightSpaceMatrix", lightSpaceMatrix);
			terrainShader.uniformMatrix4f("projMatrix", projMatrix);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, depthMap);
			drawSurface(terrainShader, terrain, rockyTerrain, Vector3f(0, 0, 0));

			if (cameraFollow) {
				viewMatrix = lookAtMatrix(viewRotation + character.position, character.position, Vector3f(0, 1.0f, 0));
			}

			if (showCoord) {
				defaultShader.bind();
				defaultShader.uniformMatrix4f("viewMatrix", viewMatrix);
				drawCoordSystem(defaultShader, Vector3f(0, 0, 0), coordVAO);
			}
			viewMatrix = lookAtMatrix(viewRotation + character.position, character.position, Vector3f(0, 1.0f, 0));

			// Processes input and swaps the window buffer
			window.update();

		}

		glDeleteFramebuffers(1, &depthMapFBO);

	}

	void detectHit() {
		if (calculateDistance(android.larmPosition, character.position) < 2.0f || calculateDistance(android.rarmPosition, character.position) < 2.0f) {
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
			Vector3f(100,0,0),
			Vector3f(0,100,0),
			Vector3f(0,0,100)
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
		case GLFW_KEY_Q:
			angle -= cam_rot_sp;
			break;
		case GLFW_KEY_E:
			angle += cam_rot_sp;
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
		Vector3f direction = viewRotation;
		float radAngle = angle * Math::PI / 180.f;
		viewRotation = Vector3f(direction.x * cosf((float)radAngle) + direction.z * sinf((float)radAngle), direction.y,
			-direction.x * sinf((float)radAngle) + direction.z * cosf((float)radAngle));
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
	float cam_rot_sp = 0.5f;

	//shadow
	GLuint depthMapFBO;
	GLuint depthMap;

	//Terrain
	Terrain terrain;

	//Images
	Image rockyTerrain;

	Cube cube_01;
	Cube cube_02;

	//Models
	Model tmp;
	Model tmp2;

	Character character;
	Android android;
	bool drawTestModel = 0;

	std::string projectPath = "";
	bool emielPC = 0;
};

int main() {
	Application app;
	app.init();
	app.update();

	return 0;
}
