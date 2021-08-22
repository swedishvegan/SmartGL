/*
	Directions:
		- Use WASD+QE to fly around and mouse to look around.
		- There are three scenes; press the numbers 1-3 to go to the corresponding scene.
		- In the second scene, you can hold down SPACE to make the physics objects fly
		and T to apply torque.
*/

#include <iostream>

// Before including SmartGL files you need to load OpenGL functions. I use GLEW for this.
#define GLEW_STATIC
#include <GL/glew.h>

// You'll also need a window management library; I use GLFW for this. This must come after the GLEW include.
#include <GLFW/glfw3.h>

#include "SmartGL.hpp"

int main() {
	
	try {

		// Just some standard GLFW setup code.
		// The GL::Exception class is just a basic form of error handling that my library uses.

		if (!glfwInit()) throw GL::Exception("Failed to initialize GLFW.");

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		const GLFWvidmode* displayInfo = glfwGetVideoMode(glfwGetPrimaryMonitor());
		int screenWidth = displayInfo->width; int screenHeight = displayInfo->height;
		
		GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "OpenGL 4.3", glfwGetPrimaryMonitor(), NULL);
		if (!window) {
			
			glfwTerminate();
			throw GL::Exception("Failed to create GLFW window.");

		}
		glfwMakeContextCurrent(window);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSwapInterval(1);

		// You must call this init function before creating any other GL objects. Otherwise, an error is generated.
		GL::init(screenWidth, screenHeight);

		// Initialize the PhysicsWorld that the PhysicsObject(s) in the second scene will belong to.

		const GL::btVec3 gravity(0.0, -24.0, 0.0);
		GL::PhysicsWorld world(gravity);

		// Load models from specially generated SmartGL model files. These files are generated from
		// "normal" model files and store the model data in a convenient format that minimizes loading
		// time. The actual model files are not included in the GitHub page since I did not create the 
		// original models.

		GL::Model die("models/die.model");
		GL::Model microphone("models/microphone.model");
		GL::Model robo("models/robo.model");
		GL::Model car("models/car.model");
		GL::Model vampire("models/vampire.model");

#define N 20
		GL::PhysicsModel* physicsModels[5 * N];

		for (int i = 0; i < N; i++) {

			int idx = 5 * i;
			btScalar height = 25.0 * (float)i;

			// Here the PhysicsModel(s) for the second scene are loaded into memory. Note that it would appear that
			// inside the loop we load the same models multiple times. Although this may seem inefficient, the engine
			// internally keeps track of which models have already been loaded and shares resources when a second copy
			// of the same model is loaded. Thus, this is the preferred way to use the same model multiple times.

			physicsModels[idx] = new GL::ConvexHullPhysicsModel("models/physicsTest0.model", world, 0.05, 1.0, GL::btVec3(0.0, height, 0.0));
			physicsModels[idx + 1] = new GL::ConvexHullPhysicsModel("models/physicsTest1.model", world, 0.05, 1.0, GL::btVec3(5.0, height + 5.0, 18.0));
			physicsModels[idx + 2] = new GL::ConvexHullPhysicsModel("models/physicsTest2.model", world, 0.05, 1.0, GL::btVec3(-5.0, height + 10.0, 19.0));
			physicsModels[idx + 3] = new GL::BoundingSpherePhysicsModel("models/physicsTest3.model", world, 0.05, 1.0, GL::btVec3(-4.0, height + 15.0, 11.0));
			physicsModels[idx + 4] = new GL::ConvexHullPhysicsModel("models/physicsTest4.model", world, 0.05, 1.0, GL::btVec3(3.0, height + 20.0, 15.0));

		}
		
		GL::GeneralPurposePhysicsModel physicsBowl("models/physicsBowl.model", world, 0.6, 0.0, GL::btVec3(0.0, -130.0, 15.0));

		// Load models for the third scene.

		GL::Model building("models/building.model");
		GL::Model ground("models/ground.model");

		// Here I just set some physical properties of the physics objects to make the collisions look prettier.

		const btScalar angularDamping = 0.3;
		const btScalar bounciness = 0.7;

		for (int i = 0; i < 5 * N; i++) {

			physicsModels[i]->getPhysicsObject().setAngularDamping(angularDamping);
			physicsModels[i]->getPhysicsObject().setBounciness(bounciness);

		}

		physicsBowl.getPhysicsObject().setBounciness(0.4);

		const char* cubeMapFilePaths[4] = { "textures/bg1.skybox", "textures/bg2.skybox", "textures/bg3.skybox", "textures/bg4.skybox" };

		// This is a convenience macro that I wrote for loading cube maps from separate files. I noticed that
		// a large chunk of the demo's startup time was devoted to loading in the cubemap textures. Thus, I
		// wrote the file Texture/TextureConverter.hpp for convenience, which converts the compressed cubemap
		// face textures into a single custom file with data in a format ready to be loaded into OpenGL.

		GL_loadCubeMapFromFile(cubeMap0, cubeMapFilePaths[0], 0, GL::DataType::F16);
		GL_loadCubeMapFromFile(cubeMap1, cubeMapFilePaths[1], 0, GL::DataType::F16);
		GL_loadCubeMapFromFile(cubeMap2, cubeMapFilePaths[2], 0, GL::DataType::F16);

		const float zNear = 1.0f;
		const float zFar = 1000.0f;

		GL::ShadowSettings settings1{ };
		settings1.numLights = 2; // Two point light shadows but no overhead shadows

		GL::ShadowSettings settings2{ };
		settings2.hasOverheadShadow = true; // No point light shadows but one overhead shadow.

		// Create the three scenes that the models will be placed in.

		GL::Scene scenes[] = {
			GL::Scene(cubeMap0, zNear, zFar),
			GL::Scene(cubeMap1, zNear, zFar, settings1),
			GL::Scene(cubeMap2, zNear, zFar, settings2),
		};

		// Bounding boxes for each scene. Ideally every model within the scene will fit within
		// the bounding box. If the scene is dynamically changing, you can update the scene's
		// bounding box every frame. You can make it as big as you want, but smaller bounding
		// boxes allow for better overhead shadow precision, so it is advantageous to make it
		// as tightly fitting as possible.

		GL::BoundingBox bb[] = {
			GL::BoundingBox(GL::vec3(-400.0f), GL::vec3(400.0f)),
			GL::BoundingBox(GL::vec3(-200.0f), GL::vec3(200.0f)),
			GL::BoundingBox(GL::vec3(-500.0f, -40.0f, -500.0f), GL::vec3(500.0f, 260.0f, 500.0f))
		};

		// Add each model to their corresponding scenes.

		scenes[0].addModel(die);
		scenes[0].addModel(microphone);
		scenes[0].addModel(robo);
		scenes[0].addModel(car);
		scenes[0].addModel(vampire);

		scenes[1].addModel(physicsBowl);
		for (int j = 0; j < 5 * N; j++) scenes[1].addModel(*physicsModels[j]);
		
		scenes[2].addModel(building);
		scenes[2].addModel(ground);

		vampire.playAnimation(0, true); // Play the vampire animation on loop.

		// Just some setup code for movement/ user input.

		const GL::vec3 up(0.0f, 1.0f, 0.0f);
		const float speedMultiplier = 0.9f;

		GL::vec3 camPos[] = { GL::vec3(0.0f), GL::vec3(-50.0f, -100.0f, -80.0f), GL::vec3(170.0f, 170.0f, 55.0f) };
		GL::vec3 camVel[3];
		for (int i = 0; i < 3; i++) camVel[i] = 0.0f;
		GL::vec3 camDir;
		GL::vec2 camRot(90.0f, 0.0f);
		GL::vec2 camRotVel(0.0f, 0.0f);
		
		float camVelMultiplier = 0.15f * speedMultiplier;
		float camVelDecay = 0.9f;
		float camRotVelMultiplier = 0.13f;
		float camRotVelDecay = 0.7f;

		double mx, my, mxLast = 0.0, myLast = 0.0;
		bool first = true;
		
		float lightStrength = 75.0f;
		float lightDistance = 300.0f;
		int bg = 0;

		double t = 0.0;

		double timeCur = glfwGetTime();
		double timeLast = timeCur;

		while (!glfwWindowShouldClose(window)) {

			// Handle movement/ user input. Skip to the next comment for engine-specific code.

			glfwPollEvents();
			if (glfwGetKey(window, GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(window, GLFW_TRUE);

			if (glfwGetKey(window, GLFW_KEY_1)) bg = 0;
			if (glfwGetKey(window, GLFW_KEY_2)) bg = 1;
			if (glfwGetKey(window, GLFW_KEY_3)) bg = 2;

			glfwGetCursorPos(window, &mx, &my);
			if (first) first = false;
			else {
				camRotVel.x += (mx - mxLast) * camRotVelMultiplier;
				camRotVel.y += (myLast - my) * camRotVelMultiplier;
			}
			camRotVel *= camRotVelDecay; camRot += camRotVel;
			if (camRot.y > 80.0f) camRot.y = 80.0f;
			if (camRot.y < -80.0f) camRot.y = -80.0f;
			mxLast = mx; myLast = my;

			camDir.x = cos(GL::radians(camRot.x)) * cos(GL::radians(camRot.y));
			camDir.y = sin(GL::radians(camRot.y));
			camDir.z = sin(GL::radians(camRot.x)) * cos(GL::radians(camRot.y));

			if (glfwGetKey(window, GLFW_KEY_W)) camVel[bg] += normalize(camDir * (1.0f - up)) * camVelMultiplier;
			if (glfwGetKey(window, GLFW_KEY_S)) camVel[bg] -= normalize(camDir * (1.0f - up)) * camVelMultiplier;
			if (glfwGetKey(window, GLFW_KEY_D)) camVel[bg] += normalize(cross(camDir, up)) * camVelMultiplier;
			if (glfwGetKey(window, GLFW_KEY_A)) camVel[bg] -= normalize(cross(camDir, up)) * camVelMultiplier;
			if (glfwGetKey(window, GLFW_KEY_E)) camVel[bg] += up * camVelMultiplier;
			if (glfwGetKey(window, GLFW_KEY_Q)) camVel[bg] -= up * camVelMultiplier;
			camVel[bg] *= camVelDecay; camPos[bg] += camVel[bg];

			camPos[bg] = max(camPos[bg], bb[bg].start);
			camPos[bg] = min(camPos[bg], bb[bg].end);

			// Apply anti-gravity and torque to physics objects based on user input. Nice little feature I added
			// for demo purposes.
			
			const GL::btVec3 torque(0.0, 50.0, 0.0);

			for (int i = 0; i < 5 * N; i++) {

				if (glfwGetKey(window, GLFW_KEY_T)) physicsModels[i]->getPhysicsObject().applyTorque(torque);
				if (glfwGetKey(window, GLFW_KEY_SPACE)) physicsModels[i]->getPhysicsObject().applyForce(gravity * -3.0 / 2.0);

			}

			timeCur = glfwGetTime();
			if (bg == 1) world.step(timeCur - timeLast); // Update the physics world.
			timeLast = timeCur;
			
			// Update the current scene's camera.

			const float FOV = GL::radians(120.0f);
			scenes[bg].updateCamera(bb[bg], camPos[bg], camPos[bg] + camDir, up, FOV);

			// Adjust the lighting settings for each scene.

			const int maxLights = 16;
			const float pi = 3.14159f;

			for (int i = 0; i < maxLights; i++) {

				double param = (double)i * (double)pi * 2.0 / (double)maxLights + t;
				GL::vec3 lightPos = GL::vec3(cos(param), -sin(2.0f * param), sin(param)) * lightDistance;
				GL::vec3 lightColor = (0.5f + 0.5f * GL::vec3(-sin(1.7f * param), cos(1.4f * param), sin(1.2f * param))) * lightStrength;

				scenes[0].setLightPosition(i, lightPos);
				scenes[0].setLightColor(i, lightColor);

			}

			scenes[1].setLightColor(0, GL::vec3(1.0f, 0.0f, 0.4f) * 320.0f);
			scenes[1].setLightPosition(0, GL::vec3(0.0f, 50.0f, -60.0f));
			scenes[1].setLightColor(1, GL::vec3(0.6f, 1.0f, 0.2f) * 170.0f);
			scenes[1].setLightPosition(1, GL::vec3(85.0f, -70.0f, 40.0f));

			scenes[2].setOverheadLightColor(GL::vec3(150.0f, 150.0f, 15.0f));
			scenes[2].setOverheadLightDirection(GL::vec3(0.6f, -1.0f, 0.7f));
			scenes[2].setBackgroundBrightness(1.8f);
			
			// Update the model matrices for each model (except the physics models, which get updated automatically).

			GL::vec3 rotationAxis(sin(t * 0.4), -cos(t * 0.5), -sin(t * 0.66));
			
			die.setModelMatrix(
				GL::translate(GL::vec3(0.0f, -20.0f, 0.0f)) *
				GL::rotate(GL::vec3(), rotationAxis, (float)t * 2.5f) *
				GL::scale(GL::vec3(std::sinf(t) * 0.08f + 0.2f))
			);

			microphone.setModelMatrix(
				GL::translate(GL::vec3(0.0f, 0.0f, -150.0f)) *
				GL::scale(GL::vec3(30.0f)) *
				GL::rotate(GL::vec3(), up, (float)t * 2.1f)
			);
			
			robo.setModelMatrix(
				GL::translate(GL::vec3(120.0f, -250.0f, 10.0f)) *
				GL::scale(GL::vec3(10.0f)) *
				GL::rotate(GL::vec3(), up, GL::radians(270.0f))
			);

			car.setModelMatrix(
				GL::translate(GL::vec3(100.0f, -80.0f, -50.0f)) *
				GL::rotate(GL::vec3(), up, (float)t * 2.5f) *
				GL::scale(GL::vec3(0.15f))
			);
			car.setMetallicValue(1.0f);
			car.setRoughnessValue(0.4f);

			vampire.setModelMatrix(
				GL::translate(GL::vec3(-85.0f, -80.0f, -50.0f)) *
				GL::scale(GL::vec3(40.0f)) *
				GL::rotate(GL::vec3(), up, GL::radians(90.0f))
			);

			building.setModelMatrix(
				GL::translate(GL::vec3(0.0f, -5.0f, 20.0f)) *
				GL::scale(GL::vec3(2.0f)) *
				GL::rotate(GL::vec3(), up, (float)t)
			);

			ground.setModelMatrix(GL::scale(GL::vec3(0.4f, 0.05f, 0.4f)));

			// Now, drawing the scene is as simple as calling the scene's draw() function :D

			scenes[bg].draw();			
			glfwSwapBuffers(window);
			
			t += 0.001;

		}

		for (int i = 0; i < 5 * N; i++) delete physicsModels[i];

	}
	catch (GL::Exception& e) { std::cout << e.getMessage() << std::endl; }
	
	glfwTerminate();
	return 0;
	
}

// Thanks for reading and have a nice day!
