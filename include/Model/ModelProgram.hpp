#ifndef MODELPROGRAM_HPP
#define MODELPROGRAM_HPP

#include <vector>

#include "./../Program/Program.hpp"
#include "./../Uniform/UniformTable.hpp"
#include "./../Texture/TextureBase.hpp"

namespace GL {

	const int ALBEDO_TEXTURE_UNIT = 0;
	const int METALLIC_ROUGHNESS_TEXTURE_UNIT = 1;
	const int NORMAL_TEXTURE_UNIT = 2;
	const int METALLIC_TEXTURE_UNIT = 6;
	const int ROUGHNESS_TEXTURE_UNIT = 7;

	struct ModelFormat {
		
		bool isAnimated;
		bool hasNormalMap;
		bool hasAlbedoMap;
		bool hasMetallicRoughnessMap;
		bool hasShadowMap;

	};

	class ModelShader : public _util {
	public:

		ModelShader(ShaderType shaderType);

		template <typename... Args>
		void init(const char* shaderSource, bool isFilePath, Args... args);

		bool isInitialized() const;

		ShaderLoader& getShader(unsigned int idx);

		ShaderType getType() const;

		~ModelShader();

	protected:

		ShaderType sType;
		ShaderLoader** loaders = nullptr;
		CodeString commonCode;
		bool isInit = false;

		void compileShader(unsigned int idx);

		static const char* macroHeaders[];
		static const char* vs_source;
		static const char* fs_source;

	};

	class ModelProgram : public _util {
	public:

		ModelProgram(ModelShader& vertexShader, ModelShader& fragmentShader);

		template <typename... Uniforms>
		void addUniforms(const char* name, UniformType dtype, unsigned int count, Uniforms... uniforms);

		template <typename... Args>
		void bindTextures(TextureBase& texture, const char* samplerName, Args... args);

		template <typename T>
		void setUniform(const char* name, T value);

		template <typename T>
		void setUniformElement(const char* name, unsigned int index, T value);

		Program& getProgram(ModelFormat format);

		UniformTable& getUniformTable();

		void prepareForUse(ModelFormat format);

		void markUnitAsOccupied(unsigned int unit, bool occupied);

		~ModelProgram();

	protected:

		Program** programs = nullptr;
		UniformTable* unis = nullptr;
		UniformTable* customUnis = nullptr;
		ModelShader* vs;
		ModelShader* fs;
		bool occupiedUnits[16];
		std::vector<TextureBase*> textures;
		std::vector<const char*> samplers;
		unsigned int numTextures = 0u;

		void linkProgram(unsigned int idx);

		void initializeUniforms();

		template <typename... Args>
		void bindTextures_impl(TextureBase& texture, const char* samplerName, Args... args);

		void bindTextures_impl(TextureBase& texture, const char* samplerName);

		static unsigned int getIndex(ModelFormat format);

	};

}

#define _GL_ModelProgram_programBase_isAnimated 1
#define _GL_ModelProgram_programBase_hasNormalMap (_GL_ModelProgram_programBase_isAnimated * 2)
#define _GL_ModelProgram_programBase_hasAlbedoMap (_GL_ModelProgram_programBase_hasNormalMap * 2)
#define _GL_ModelProgram_programBase_hasMetallicRoughnessMap (_GL_ModelProgram_programBase_hasAlbedoMap * 2)
#define _GL_ModelProgram_programBase_hasShadowMap (_GL_ModelProgram_programBase_hasMetallicRoughnessMap * 2)
#define _GL_ModelProgram_numPrograms (_GL_ModelProgram_programBase_hasShadowMap * 2)
#define _GL_ModelProgram_numMacroOptions 6

template <typename... Args>
void GL::ModelShader::init(const char* shaderSource, bool isFilePath, Args... args) { 

	commonCode.init(shaderSource, isFilePath, args...);
	isInit = true;
	
}

template <typename... Uniforms>
void GL::ModelProgram::addUniforms(const char* name, GL::UniformType dtype, unsigned int count, Uniforms... uniforms) {

	if (customUnis) throw Exception("Attempt to initialize custom uniform table twice in ModelProgram.");

	customUnis = new UniformTable();
	customUnis->init(name, dtype, count, uniforms...);

}

template <typename... Args>
void GL::ModelProgram::bindTextures(GL::TextureBase& texture, const char* samplerName, Args... args) { 
	
	if (!customUnis) throw Exception("Textures cannot be bound to a ModelProgram until the uniform names for their corresponding samplers have been declared.");

	numTextures = 0u; 
	bindTextures_impl(texture, samplerName, args...);

}

template <typename T>
void GL::ModelProgram::setUniform(const char* name, T value) { 
	
	initializeUniforms();
	for (int i = 0; i < _GL_ModelProgram_numPrograms; i++) customUnis->set<T>(name, value);

}

template <typename T>
void GL::ModelProgram::setUniformElement(const char* name, unsigned int index, T value) { 
	
	initializeUniforms();
	for (int i = 0; i < _GL_ModelProgram_numPrograms; i++) customUnis->setElement<T>(name, index, value);

}

template <typename... Args>
void GL::ModelProgram::bindTextures_impl(GL::TextureBase& texture, const char* samplerName, Args... args) {

	bindTextures_impl(texture, samplerName);
	bindTextures_impl(args...);

}

#endif