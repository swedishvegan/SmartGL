#ifndef CUBEMAPCONVOLUTION_HPP
#define CUBEMAPCONVOLUTION_HPP

#include "./../Texture/TextureCubeMap.hpp"
#include "./../Program/Program.hpp"
#include "./../Framebuffer/Framebuffer.hpp"
#include "./../Uniform/UniformTable.hpp"
#include "./../VertexArray/VertexArray.hpp"

#define _MakeConvolution(ClassName, defaultUnit, defaultDim, privateMembers) \
template <typename S> \
class ClassName : public CubeMapConvolution<S> { \
public: \
	\
	ClassName(TextureCubeMap<S>& cubeMap, unsigned int unit = defaultUnit, unsigned int dim = defaultDim, bool autoGen = true); \
	\
	void regenerate() const; \
	\
	~ClassName(); \
	\
protected: \
	\
	privateMembers \
	\
};

namespace GL {

	template <typename S>
	class CubeMapConvolution : public TextureCubeMap<S> {
	public:

		virtual void regenerate() const = 0;

	protected:

		GLuint parentID;
		unsigned int parentUnit;

		CubeMapConvolution(TextureCubeMap<S>& cubeMap, unsigned int unit, unsigned int dim);

	};

	_MakeConvolution(IrradianceCubeMap, 3u, 64u, Framebuffer* fb = nullptr;);

	_MakeConvolution(SpecularCubeMap, 4u, 512u, GLuint fb = 0u;);

}

template <typename S>
GL::CubeMapConvolution<S>::CubeMapConvolution(TextureCubeMap<S>& cubeMap, unsigned int unit, unsigned int dim) : Texture(GL_TEXTURE_CUBE_MAP, unit, cubeMap.getColorFormat(), cubeMap.getInternalDataType()), TextureCubeMap<S>(dim, unit, cubeMap.getColorFormat(), cubeMap.getInternalDataType()) {

	if (unit == parentUnit) throw Exception("A cube map convolution cannot have the same texture unit (" + std::to_string(unit) + ") as the original cube map.");

	parentID = cubeMap.getID();
	parentUnit = cubeMap.getUnit();

	if (Texture::isInt) throw Exception("A cube map convolution cannot be performed with an integer GPU data type.");
	if (cubeMap.getColorFormat() != ColorFormat::RGB) throw Exception("A cube map convolution expects a cube map with RGB format as input.");

}

template <typename S>
GL::IrradianceCubeMap<S>::IrradianceCubeMap(TextureCubeMap<S>& cubeMap, unsigned int unit, unsigned int dim, bool autoGen) : Texture(GL_TEXTURE_CUBE_MAP, unit, cubeMap.getColorFormat(), cubeMap.getInternalDataType()), CubeMapConvolution<S>(cubeMap, unit, dim) {

	if (!_util::cubeMapIrradianceProgram) {

		ShaderLoader* vertShader = (_util::cubeMapCommonShaderLoader) ? (ShaderLoader*)(_util::cubeMapCommonShaderLoader) : new ShaderLoader(ShaderType::VERTEX);
		if (!vertShader->isInitialized()) vertShader->init(_util::cubeMapIrradianceProgramSource[0], false);

		ShaderLoader fragShader(ShaderType::FRAGMENT);
		fragShader.init(_util::cubeMapIrradianceProgramSource[1], false);

		Program* prog = new Program();
		prog->init(*vertShader, fragShader);

		UniformTable* ut = new UniformTable(*prog);
		ut->init(
			"rot", UniformType::MAT3, 1,
			"inputCubeMap", UniformType::INT, 1
		);

		_util::cubeMapIrradianceProgram = (void*)prog;
		_util::cubeMapIrradianceUniforms = (void*)ut;
		if (!_util::cubeMapCommonShaderLoader) _util::cubeMapCommonShaderLoader = (void*)vertShader;

	}

	fb = new Framebuffer(dim, dim);
	if (autoGen) regenerate();

}

template <typename S>
void GL::IrradianceCubeMap<S>::regenerate() const {
	
	Program* prog = (Program*)_util::cubeMapIrradianceProgram;
	UniformTable* ut = (UniformTable*)_util::cubeMapIrradianceUniforms;

	prog->use();
	ut->set<int>("inputCubeMap", (int)CubeMapConvolution<S>::parentUnit);
	fb->bind();

	glActiveTexture(GL_TEXTURE0 + CubeMapConvolution<S>::parentUnit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, CubeMapConvolution<S>::parentID);

	glActiveTexture(GL_TEXTURE0 + Texture::unit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, Texture::ID);

	for (int i = 0; i < 6; i++) {

		ut->set("rot", _util::cubeMap_rotationMatrices[i]);
		ut->update();

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, Texture::ID, 0);

		Render::clearBuffers(COLOR_BUFFER);
		Render::drawDefault(0u, 6u);

	}
	
}

template <typename S>
GL::IrradianceCubeMap<S>::~IrradianceCubeMap() { if (fb) delete fb; }

template <typename S>
GL::SpecularCubeMap<S>::SpecularCubeMap(TextureCubeMap<S>& cubeMap, unsigned int unit, unsigned int dim, bool autoGen) : Texture(GL_TEXTURE_CUBE_MAP, unit, cubeMap.getColorFormat(), cubeMap.getInternalDataType()), CubeMapConvolution<S>(cubeMap, unit, dim) {

	if (dim != 256u && dim != 512u && dim != 1024u) throw Exception("A specular cube map dimension must either be 256, 512, or 1024, but not " + std::to_string(dim) + ".");

	if (!_util::cubeMapSpecularProgram) {

		ShaderLoader* vertShader = (_util::cubeMapCommonShaderLoader) ? (ShaderLoader*)(_util::cubeMapCommonShaderLoader) : new ShaderLoader(ShaderType::VERTEX);
		if (!vertShader->isInitialized()) vertShader->init(_util::cubeMapIrradianceProgramSource[0], false);

		ShaderLoader fragShader(ShaderType::FRAGMENT);
		fragShader.init(
			_util::cubeMapSpecularProgramSource[0], false,
			_util::importanceSampleCommonCode, false,
			_util::cubeMapSpecularProgramSource[1], false
		);

		Program* prog = new Program();
		prog->init(*vertShader, fragShader);

		UniformTable* ut = new UniformTable(*prog);
		ut->init(
			"rot", UniformType::MAT3, 1,
			"roughness", UniformType::FLOAT, 1,
			"resolution", UniformType::FLOAT, 1,
			"inputCubeMap", UniformType::INT, 1
		);

		_util::cubeMapSpecularProgram = (void*)prog;
		_util::cubeMapSpecularUniforms = (void*)ut;
		if (!_util::cubeMapCommonShaderLoader) _util::cubeMapCommonShaderLoader = (void*)vertShader;

	}

	Texture::bind();
	Texture::updateMipmaps();
	Texture::setMinFilter(TextureFilter::LINEAR, TextureFilter::LINEAR);

	glGenFramebuffers(1, &fb);
	if (autoGen) regenerate();

}

template <typename S>
void GL::SpecularCubeMap<S>::regenerate() const {

	Program* prog = (Program*)_util::cubeMapSpecularProgram;
	UniformTable* ut = (UniformTable*)_util::cubeMapSpecularUniforms;

	prog->use();
	ut->set<int>("inputCubeMap", (int)CubeMapConvolution<S>::parentUnit);
	ut->set<float>("resolution", (float)TextureCubeMap<S>::dim);

	glBindFramebuffer(GL_FRAMEBUFFER, fb);

	glActiveTexture(GL_TEXTURE0 + CubeMapConvolution<S>::parentUnit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, CubeMapConvolution<S>::parentID);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	glActiveTexture(GL_TEXTURE0 + Texture::unit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, Texture::ID);

	const unsigned int maxMipLevels = 5u;
	unsigned int curDim = TextureCubeMap<S>::dim;

	for (unsigned int level = 0u; level < maxMipLevels; level++) {

		float roughness = (float)level / (float)(maxMipLevels - 1);
		ut->set("roughness", roughness);

		glViewport(0, 0, curDim, curDim);

		for (int i = 0; i < 6; i++) {

			ut->set("rot", _util::cubeMap_rotationMatrices[i]);
			ut->update();

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, Texture::ID, level);

			glClear(GL_COLOR_BUFFER_BIT);
			VertexArray<>::drawDefault(0u, 6u);

		}

		curDim /= 2u;

	}

}

template <typename S>
GL::SpecularCubeMap<S>::~SpecularCubeMap() { if (fb) glDeleteFramebuffers(1, &fb); }

#endif