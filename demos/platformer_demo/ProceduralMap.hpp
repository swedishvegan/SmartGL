#pragma once

#include "SmartGL.hpp"
using namespace GL;

class ProceduralMap {
public:

	ProceduralMap(uint unit, const char* sourceCode, TextureFilter filterMode);

	void bind();

protected:

	Texture3D map;
	Program program;

};

class MetallicMap : public ProceduralMap {
public: MetallicMap() : ProceduralMap(METALLIC_TEXTURE_UNIT, sourceCode, TextureFilter::LINEAR) { }
protected: static const char* sourceCode;
};

class RoughnessMap : public ProceduralMap {
public: RoughnessMap() : ProceduralMap(ROUGHNESS_TEXTURE_UNIT, sourceCode, TextureFilter::LINEAR) { }
protected: static const char* sourceCode;
};

ProceduralMap::ProceduralMap(uint unit, const char* sourceCode, TextureFilter filterMode) : map(256, 256, 256, unit, ColorFormat::R, DataType::F16) {

	ShaderLoader compShader(ShaderType::COMPUTE);
	compShader.init(CUR_DIRECTORY "/shaders/noise_renderer.comp", true, sourceCode, false);
	program.init(compShader);
	map.bindToImageUnit(0);
	map.setMinFilter(filterMode);
	map.setMagFilter(filterMode);
	map.setWrapMode(TextureWrap::REPEAT);
	program.dispatchCompute(uvec3(256u / 8u));

}

void ProceduralMap::bind() { map.bind(); }

const char* MetallicMap::sourceCode = \
"float getTextureValue(vec3 pos) { \
	\
	float m = compoundNoise(-pos + vec3(2.3456)); \
	return 1.0 - m * m; \
	\
}";

const char* RoughnessMap::sourceCode = \
\
"vec3 moveToQuad(vec3 pos, vec3 signs) { return vec3(0.5) + signs * (pos - vec3(0.5)); } \
\
float getTextureValue(vec3 pos) { \
	\
	pos = fract(pos); \
	float l = length(vec3(0.5) - pos); \
	return clamp(((l - 0.5) * 35.0) * 0.5 + 0.5, 0.0, 1.0); \
	\
}";
