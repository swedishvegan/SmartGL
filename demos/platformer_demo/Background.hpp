#pragma once

#include "SmartGL.hpp"

using namespace GL;

class Background : public _util {
public:

	Background(uint cubeMapDim, uint unit);

	void bind();

	ImageTextureCubeMap& getCubeMap();

private:

	ImageTextureCubeMap cubeMap;
	Framebuffer framebuffer;
	static Program* program;
	static UniformTable* unis;

};

Program* Background::program = nullptr;
UniformTable* Background::unis = nullptr;

Background::Background(uint cubeMapDim, uint unit) : cubeMap(cubeMapDim, unit, ColorFormat::RGB, DataType::F16), framebuffer(cubeMapDim, cubeMapDim) {

	for (int i = 0; i < 6; i++) framebuffer.setColorTarget(cubeMap, (CubeMapFace)i, i);
	
	if (!program) {

		ShaderLoader vertShader(ShaderType::VERTEX);
		vertShader.init("shaders/background.vert", true);

		ShaderLoader fragShader(ShaderType::FRAGMENT);
		fragShader.init("shaders/background.frag", true);

		program = new Program();
		program->init(vertShader, fragShader);

		unis = new UniformTable(*program);
		unis->init(
			"rot", UniformType::MAT3, 1
		);

	}

	program->use();

	for (int i = 0; i < 6; i++) {

		unis->set<mat3>("rot", cameraRotation(_util::cubeMap_lookAt_targetVectors[i], _util::cubeMap_lookAt_upVectors[i]));
		unis->update();

		framebuffer.use(i);
		Render::drawDefault();

	}

}

void Background::bind() { cubeMap.bind(); }

ImageTextureCubeMap& Background::getCubeMap() { return cubeMap; }