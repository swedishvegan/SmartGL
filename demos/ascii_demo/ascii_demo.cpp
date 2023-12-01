
#include <iostream>

#include "SmartGL.hpp"
#include "FontConverter.hpp"

#include <GLFW/glfw3.h>

#define WINDOWED

int main() {

	try {

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

		GL::FontLoader font_loader(CUR_DIRECTORY "/fonts/timesnewroman.ttf", 30, 169); // Load font from file (30 pt, 169 ppi)
		GL::Font font(font_loader);

		FontConverter converter(font_loader, GL::uvec2(screenWidth / 2, screenHeight), 64, 6); // Used to convert a texture to ASCII

		FontConverter::Settings converter_settings;
		converter_settings.normalizeColors = false;
		//converter_settings.correctiveExponent = 0.5;

		// Initialize utilities for rendering split screen

		GL::ColorRenderTexture render_target_lh(screenWidth / 2, screenHeight, 0u);
		GL::DepthStencilRenderbuffer depth_buffer(screenWidth / 2, screenHeight);

		GL::Framebuffer framebuffer_lh(screenWidth / 2, screenHeight);
		framebuffer_lh.setColorTarget(render_target_lh);
		framebuffer_lh.setDepthStencilTarget(depth_buffer);

		GL::ColorRenderTexture render_target_rh(screenWidth / 2, screenHeight, 1u);

		GL::Framebuffer framebuffer_rh(screenWidth / 2, screenHeight);
		framebuffer_rh.setColorTarget(render_target_rh);

		GL_LoadProgramFrom(splitscreen, CUR_DIRECTORY "/shaders/");

		GL::UniformTable splitscreen_unis(splitscreen);
		splitscreen_unis.init(
			"lh", GL::UniformType::SAMPLER, 1,
			"rh", GL::UniformType::SAMPLER, 1
		);

		splitscreen_unis.set<int>("lh", 0);
		splitscreen_unis.set<int>("rh", 1);

		splitscreen_unis.update();

		GL_loadCubeMapFromFile(background, CUR_DIRECTORY "/cubemaps/spacebox1.cubemap", 0u, GL::DataType::F16);

		const float zNear = 1.0f;
		const float zFar = 1000.0f;

		GL::ShadowSettings shadowSettings{ };
		//shadowSettings.numLights = 1;

		GL::Scene scene(GL::uvec2(screenWidth / 2, screenHeight), background, zNear, zFar, shadowSettings);
		scene.setBackgroundBrightness(0.01);

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
		scene.setLightPosition(0, GL::vec3(-40.0, 60.0, 0.0));
		scene.setLightColor(0, GL::vec3(20.0, 20.0, 10.0));

		const float bbSize = 50.0;
		GL::BoundingBox bb{ GL::vec3(-bbSize), GL::vec3(bbSize) };

		GL::vec3 camPos;
		GL::vec3 up(0.0, 1.0, 0.0);

		const float camRadius = 30.0;
		float spin = 0.0f;

		while (!glfwWindowShouldClose(window)) {

			glfwPollEvents();
			if (glfwGetKey(window, GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(window, GLFW_TRUE);

			camPos = GL::normalize(GL::vec3(cos(spin), 0.0, sin(spin))) * camRadius;
			scene.updateCamera(bb, camPos, GL::vec3(), up, GL::radians(90.0f));

			scene.draw(framebuffer_lh); // Draw the scene to the left side of the page
			font.write(framebuffer_lh, "Hello, world!", GL::vec2(50, 50));

			converter.convert<GL::ColorRenderTexture>(render_target_lh, framebuffer_rh, converter_settings); // Render the scene in ASCII on the right side

			splitscreen.use();
			render_target_lh.bind();
			render_target_rh.bind();

			GL::Framebuffer::useDefault();
			GL::Render::clearBuffers();
			GL::Render::drawDefault();
			glfwSwapBuffers(window);

			if (!glfwGetKey(window, GLFW_KEY_P)) spin += 0.01;

		}
		
	}
	catch (GL::Exception& e) { std::cout << e.getMessage() << std::endl; }

	glfwTerminate();
	return 0;
	
}