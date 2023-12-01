#include <iostream>

#include "SmartGL.hpp"

#include <GLFW/glfw3.h>

using namespace GL;

#include "Background.hpp"
#include "ProceduralMap.hpp"

//#define WINDOWED

int main() {

	try {

		if (!glfwInit()) throw Exception("Failed to initialize GLFW.");

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
			throw Exception("Failed to create GLFW window.");

		}
		glfwMakeContextCurrent(window);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSwapInterval(1);

		init(screenWidth, screenHeight);

		MetallicMap metallicMap;
		RoughnessMap roughnessMap;

		SampleSettings customSettings{ };
		customSettings.metallicTex3D = true;
		customSettings.roughnessTex3D = true;
		
		Background background(2048, 0);
		
		const btScalar gravity = -300.0;
		PhysicsWorld physicsWorld(btVec3(0.0, gravity, 0.0));

		const btVec3 sphereSpawnPoint(0.0, 45.0, 0.0);

		ModelShader customVertShader(ShaderType::VERTEX);
		ModelShader customFragShader(ShaderType::FRAGMENT);

		customVertShader.init(CUR_DIRECTORY "/shaders/custom.vert", true);
		customFragShader.init(CUR_DIRECTORY "/shaders/custom.frag", true);

		ModelProgram customProgram(customVertShader, customFragShader);
		customProgram.addUniforms("skybox", UniformType::SAMPLER, 1);
		customProgram.bindTextures(background.getCubeMap(), "skybox");

		BoundingSpherePhysicsModel basic_sphere(CUR_DIRECTORY "/models/basic_sphere.model", physicsWorld, 0.04, 2.0, false, sphereSpawnPoint);
		GeneralPurposePhysicsModel basic_floor(CUR_DIRECTORY "/models/basic_floor.model", physicsWorld, 0.08, 0.0, true);
		
		basic_sphere.setColor(vec3(1.0f, 0.933f, 0.180f));
		basic_sphere.getPhysicsObject().setAngularDamping(0.3);
		basic_sphere.getPhysicsObject().setBounciness(1.0);
		//basic_sphere.setCustomProgram(customProgram);

		basic_floor.setColor(vec3(0.0f, 1.0f, 0.0f));
		basic_floor.getPhysicsObject().setPosition(basic_floor.getPhysicsObject().getPosition() + btVec3(0.0, 5.0, 0.0));
		basic_floor.getPhysicsObject().setBounciness(0.25);
		basic_floor.setMetallicValue(0.0);
		basic_floor.setRoughnessValue(0.4);
		basic_floor.setShadowOcclusion(0.2);
		basic_floor.setSamplingFactor3D(TextureType::METALLIC, 0.00027);
		basic_floor.setSamplingFactor3D(TextureType::ROUGHNESS, 0.002);
		//basic_floor.setCustomProgram(customProgram);
		
		ShadowSettings shadowSettings{ };
		shadowSettings.hasOverheadShadow = true;
		//shadowSettings.overheadShadowFancy = true;
		
		Scene scene(background.getCubeMap(), 4.0, 2000.0, shadowSettings, 1024);
		scene.setBackgroundColor(vec3(0.3f, 0.3f, 0.5f));
		scene.setOverheadLightColor(25.0f * vec3(5.0f, 2.0f, 1.0f));
		scene.setOverheadLightDirection(vec3(1.0f, -0.7f, 0.5f));
		scene.addModel(basic_sphere);
		scene.addModel(basic_floor);
		//scene.setModelShouldCastShadow(basic_floor, false);
		scene.applySampleSettings(basic_floor, customSettings);
		//scene.setBackgroundBrightness(1.6);

		vec3 spherePosDrag = basic_sphere.getPhysicsObject().getPosition();
		const float spherePosDragSpeed = 5.0;
		
		float xRot = 0.0;
		float yRotOffset = 0.0;
		const float yRotOffsetMax = 0.4;
		const float yRot = 0.55;
		const float rotVel = 0.05;

		float xRotDrag = xRot;
		float yRotDrag = yRot;
		const float rotDragSpeed = 12.0;
		
		const double tiltMax = 1.2;
		const double tiltDragSpeed = 4.0;

		float viewRad = 140.0;

		double springDisplacement = 0.0;
		const double springPullMax = -17.0;
		const double springDragSpeed = 9.0;

		const double floorDamping = 0.6;
		const double airDamping = 0.15;

		double lastTime = glfwGetTime();
		bool jumping = false;
		//double mxLast, myLast;
		//bool mouseFirst = true;

		while (!glfwWindowShouldClose(window)) {

			glfwPollEvents();
			if (glfwGetKey(window, GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(window, GLFW_TRUE);

			if (glfwGetKey(window, GLFW_KEY_RIGHT)) xRot += rotVel;
			if (glfwGetKey(window, GLFW_KEY_LEFT)) xRot -= rotVel;

			if (glfwGetKey(window, GLFW_KEY_UP)) yRotOffset -= rotVel;
			if (glfwGetKey(window, GLFW_KEY_DOWN)) yRotOffset += rotVel;

			double curTime = glfwGetTime();
			double deltaTime = curTime - lastTime;

			/*double mx, my;
			glfwGetCursorPos(window, &mx, &my);

			if (mouseFirst) {

				mxLast = mx;
				myLast = my;
				mouseFirst = false;

			}

			vec2 mouseDelta = vec2(mx - mxLast, my - myLast) * rotVel;
			xRot += mouseDelta.x; 
			yRotOffset += mouseDelta.y;*/
			yRotOffset = clamp(yRotOffset, -yRotOffsetMax, yRotOffsetMax);
			float finalYRot = yRot + yRotOffset;

			xRotDrag += (xRot - xRotDrag) * deltaTime * rotDragSpeed;
			yRotDrag += (finalYRot - yRotDrag) * deltaTime * rotDragSpeed;

			if (glfwGetKey(window, GLFW_KEY_R)) basic_sphere.getPhysicsObject().teleport(sphereSpawnPoint);

			//if (glfwGetKey(window, GLFW_KEY_UP)) yRotOffset += yRotVel;
			//if (glfwGetKey(window, GLFW_KEY_DOWN)) yRotOffset -= yRotVel;

			CollisionData data;
			physicsWorld.getCollisionData(basic_sphere.getPhysicsObject(), basic_floor.getPhysicsObject(), data);

			dvec3 tilt = 0.0;

			if (data.numContacts > 0) {

				if (glfwGetKey(window, GLFW_KEY_W)) tilt.y = 1.0;
				if (glfwGetKey(window, GLFW_KEY_S)) tilt.y = -1.0;
				if (glfwGetKey(window, GLFW_KEY_D)) tilt.x = 1.0;
				if (glfwGetKey(window, GLFW_KEY_A)) tilt.x = -1.0;

			}

			dvec3 normTilt = normalize(tilt) * tiltMax;

			double damping = (data.numContacts == 0) ? airDamping : floorDamping;
			basic_sphere.getPhysicsObject().setLinearDamping(damping);

			bool spacePressed = glfwGetKey(window, GLFW_KEY_SPACE);

			if (spacePressed) { if (!jumping && data.numContacts > 0) jumping = true; }
			else jumping = false;

			vec3 camPos;
			camPos.x = cos(xRotDrag) * cos(yRotDrag);
			camPos.y = sin(yRotDrag);
			camPos.z = sin(xRotDrag) * cos(yRotDrag);

			vec3 gravityVecY = vec3(0.0, 1.0, 0.0);
			vec3 right = normalize(cross(gravityVecY, camPos));
			vec3 forward = normalize(cross(right, gravityVecY));

			vec3 gravityVecX = rotate(vec3(), forward, (float)normTilt.x) * upscale<vec4>(gravityVecY);
			gravityVecY = rotate(vec3(), right, (float)normTilt.y) * upscale<vec4>(gravityVecY);

			vec3 gravityVec = normalize(gravityVecX + gravityVecY);
			physicsWorld.setGravity(btVec3(gravityVec) * gravity);

			double springPull = 0.0;
			if (jumping) springPull = springPullMax;
			springDisplacement += (springPull - springDisplacement) * deltaTime * springDragSpeed;
			
			basic_floor.getPhysicsObject().setPosition(btVec3(0.0, springDisplacement, 0.0));

			physicsWorld.step(deltaTime, 240.0);
			lastTime = curTime;
			
			vec3 spherePos(basic_sphere.getPhysicsObject().getPosition());
			spherePosDrag += (spherePos - spherePosDrag) * spherePosDragSpeed * (float)deltaTime;
			camPos = spherePosDrag + camPos * viewRad;
			
			scene.updateCamera(/*BoundingBox(spherePos - vec3(viewRad), spherePos + vec3(viewRad)), */scene.getBoundingBoxApproximation(), camPos, spherePosDrag, vec3(0.0f, 1.0f, 0.0f), radians(60.0));
			metallicMap.bind();
			roughnessMap.bind();
			scene.draw();
			glfwSwapBuffers(window);

			//mxLast = mx; myLast = my;
			
		}

	}
	catch (Exception& e) { std::cout << e.getMessage() << std::endl; }

	glfwTerminate();
	return 0;

}