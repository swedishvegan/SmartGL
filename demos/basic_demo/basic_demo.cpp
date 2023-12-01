
#include <iostream>

#include "SmartGL.hpp"

// GLFW to create a window (Note: SmartGL must be included BEFORE window library)
#include <GLFW/glfw3.h>

#define WINDOWED

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

		// You must call this init function before creating any other GL objects. Otherwise, an error is generated.
		GL::init(screenWidth, screenHeight);

		GL_loadCubeMapFromFile(background, CUR_DIRECTORY "/cubemaps/spacebox1.cubemap", 0u, GL::DataType::F16);

		const float zNear = 1.0f;
		const float zFar = 1000.0f;

		GL::ShadowSettings shadowSettings{ };
		//shadowSettings.numLights = 1;

		GL::Scene scene(background, zNear, zFar, shadowSettings);

		GL::Model amongus(CUR_DIRECTORY "/models/amongus.model");
		amongus.setModelMatrix(GL::translate(GL::vec3(0.0, -15.0, 0.0)) * GL::scale(GL::vec3(0.15f)));
		amongus.setMetallicValue(0.6);
		amongus.setRoughnessValue(0.3);

		GL::Model monsterfish(CUR_DIRECTORY "/models/monsterfish.model");
		monsterfish.setModelMatrix(GL::translate(GL::vec3(0.0, -25.0, -10.0)) * GL::scale(GL::vec3(0.4f)));

		GL::Model penguin(CUR_DIRECTORY "/models/penguin.model");
		penguin.setModelMatrix(
			GL::translate(GL::vec3(20.0, -20.0, 0.0)) * 
			GL::scale(GL::vec3(20.0f)) * 
			GL::rotate(GL::vec3(), GL::vec3(0.0, 1.0, 0.0), GL::radians(90.0f))
		);

		scene.addModel(amongus);
		scene.addModel(monsterfish);
		scene.addModel(penguin);

		scene.setBackgroundBrightness(2.0);
		scene.setLightPosition(0, GL::vec3(-25.0, 40.0, 0.0));
		scene.setLightColor(0, GL::vec3(20.0, 20.0, 10.0));

		const float bbSize = 50.0;
		GL::BoundingBox bb{ GL::vec3(-bbSize), GL::vec3(bbSize) };

		GL::vec3 camPos;
		GL::vec3 up(0.0, 1.0, 0.0);

		const float camRadius = 80.0;
		float spin = 0.0f;

		while (!glfwWindowShouldClose(window)) {

			glfwPollEvents();
			if (glfwGetKey(window, GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(window, GLFW_TRUE);

			camPos = GL::normalize(GL::vec3(cos(spin), 0.0, sin(spin))) * camRadius * (0.25f * sinf(spin * 1.8) + 0.75f);
			scene.updateCamera(bb, camPos, GL::vec3(), up, GL::radians(90.0f));

			scene.draw();
			glfwSwapBuffers(window);

			if (!glfwGetKey(window, GLFW_KEY_P)) spin += 0.01;

		}
		
	}
	catch (GL::Exception& e) { std::cout << e.getMessage() << std::endl; }

	glfwTerminate();
	return 0;
	
}