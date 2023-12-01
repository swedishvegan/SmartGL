
#include <iostream>

#include "SmartGL.hpp"

// GLFW to create a window
#include <GLFW/glfw3.h>

#define WINDOWED

int main() {

	try {

		// GLFW setup code.

		if (!glfwInit()) throw GL::Exception("Failed to initialize GLFW.");

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		const GLFWvidmode* displayInfo = glfwGetVideoMode(glfwGetPrimaryMonitor());

#ifdef WINDOWED
		int screenWidth = 1600; int screenHeight = 900;
		GLFWmonitor* monitor = nullptr;
#else
		int screenWidth = displayInfo->width; int screenHeight = displayInfo->height;
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
#endif

		GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "OpenGL 4.3", monitor, NULL);
		if (!window) {

			glfwTerminate();
			throw GL::Exception("Failed to create GLFW window.");

		}
		glfwMakeContextCurrent(window);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		glfwSwapInterval(1);

		GL::init(screenWidth, screenHeight);

		// Initialize the PhysicsWorld that the PhysicsObject(s) in the second scene will belong to.

		const GL::btVec3 gravity(0.0, -24.0, 0.0);
		GL::PhysicsWorld world(gravity);

		GL_loadCubeMapFromFile(background, CUR_DIRECTORY "/cubemaps/Spacebox3/spacebox3.cubemap", 0u, GL::DataType::F16);

		const float zNear = 1.0f;
		const float zFar = 1000.0f;

		GL::ShadowSettings shadowSettings{ };
		shadowSettings.numLights = 2;

		GL::Scene scene(background, zNear, zFar, shadowSettings);

		GL::GeneralPurposePhysicsModel bowl(CUR_DIRECTORY "/models/bowl.model", world, 0.3, 0.0, false, GL::btVec3(0.0, -20.0, 0.0));

		#define N 20
		GL::PhysicsModel* physicsModels[3 * N];

		for (int i = 0; i < N; i++) {

			int idx = 3 * i;
			btScalar height = 25.0 * (float)i;

			// Here the PhysicsModel(s) for the second scene are loaded into memory. Note that it would appear that
			// inside the loop we load the same models multiple times. Although this may seem inefficient, the engine
			// internally keeps track of which models have already been loaded and shares resources when a second copy
			// of the same model is loaded. Thus, this is the preferred way to use the same model multiple times.

			physicsModels[idx] = new GL::ConvexHullPhysicsModel(CUR_DIRECTORY "/models/ball.model", world, 0.05, 1.0, false, GL::btVec3(0.0, height, 0.0));
			physicsModels[idx + 1] = new GL::ConvexHullPhysicsModel(CUR_DIRECTORY "/models/egg.model", world, 0.02, 1.0, false, GL::btVec3(5.0, height + 5.0, 18.0));
			physicsModels[idx + 2] = new GL::ConvexHullPhysicsModel(CUR_DIRECTORY "/models/cone.model", world, 0.05, 1.0, false, GL::btVec3(-5.0, height + 10.0, 19.0));

			physicsModels[idx]->setColor(GL::vec3(5.0, 0.0, 0.0));
			physicsModels[idx + 1]->setColor(GL::vec3(5.0, 5.0, 0.0));
			physicsModels[idx + 2]->setColor(GL::vec3(0.5, 0.5, 5.0));

		}

		const btScalar angularDamping = 0.3;
		const btScalar bounciness = 0.7;

		for (int i = 0; i < 3 * N; i++) {

			physicsModels[i]->getPhysicsObject().setAngularDamping(angularDamping);
			physicsModels[i]->getPhysicsObject().setBounciness(bounciness);

		}

		bowl.getPhysicsObject().setBounciness(0.4);

		scene.addModel(bowl);
		for (int i = 0; i < 3 * N; i++) scene.addModel(*physicsModels[i]);

		scene.setBackgroundBrightness(2.0);
		scene.setLightPosition(0, GL::vec3(-80.0, 80.0, 0.0));
		scene.setLightColor(0, GL::vec3(40.0, 5.0, 5.0));
		scene.setLightPosition(1, GL::vec3(30.0, 30.0, 90.0));
		scene.setLightColor(1, GL::vec3(5.0, 5.0, 40.0));

		const float bbSize = 50.0;
		GL::BoundingBox bb{ GL::vec3(-bbSize), GL::vec3(bbSize) };

		GL::vec3 camPos;
		GL::vec3 up(0.0, 1.0, 0.0);

		const float camRadius = 165.0;
		const float heightIncrement = 1.0;
		float height = 5.0f;
		const float spinIncrement = 0.01;
		float spin = 0.0f;

		double timeCur = glfwGetTime();
		double timeLast = timeCur;

		while (!glfwWindowShouldClose(window)) {

			// glfw event handling

			glfwPollEvents();
			if (glfwGetKey(window, GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(window, GLFW_TRUE);

			if (glfwGetKey(window, GLFW_KEY_LEFT)) spin -= spinIncrement;
			if (glfwGetKey(window, GLFW_KEY_RIGHT)) spin += spinIncrement;
			if (glfwGetKey(window, GLFW_KEY_UP)) height += heightIncrement;
			if (glfwGetKey(window, GLFW_KEY_DOWN)) height -= heightIncrement;

			// Apply anti-gravity and torque to physics objects based on user input

			const GL::btVec3 torque(0.0, 50.0, 0.0);

			for (int i = 0; i < 3 * N; i++) {

				if (glfwGetKey(window, GLFW_KEY_T)) physicsModels[i]->getPhysicsObject().applyTorque(torque);
				if (glfwGetKey(window, GLFW_KEY_SPACE)) physicsModels[i]->getPhysicsObject().applyForce(gravity * -3.0f / 2.0f);

			}

			timeCur = glfwGetTime();
			if (!glfwGetKey(window, GLFW_KEY_P)) world.step(timeCur - timeLast); // Update the physics world
			timeLast = timeCur;

			camPos = GL::normalize(GL::vec3(cos(spin), 0.0, sin(spin))) * camRadius;
			scene.updateCamera(bb, camPos + GL::vec3(0.0, height, 0.0), GL::vec3(), up, GL::radians(90.0f)); // Update camera position

			scene.draw();
			glfwSwapBuffers(window); // Render

		}

		for (int i = 0; i < 3 * N; i++) delete physicsModels[i];

	}
	catch (GL::Exception& e) { std::cout << e.getMessage() << std::endl; }

	glfwTerminate();
	return 0;
	
}