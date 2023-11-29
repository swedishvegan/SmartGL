#ifndef SHADERLOADER_HPP
#define SHADERLOADER_HPP

#include "./../util/util.hpp"
#include "./../util/enums.hpp"
#include "./../util/GL-math.hpp"
#include "./CodeString.hpp"

namespace GL {

	class ShaderLoader : public _util {
	public:

		ShaderLoader(ShaderType shaderType);

		ShaderLoader(const ShaderLoader& copy);

		bool isInitialized() const;

		template <typename... Args>
		void init(const char* shaderSource, bool isFilePath, Args... args);

		void init(char** shaderSource, unsigned int length);

		GLuint getID() const;

		ShaderType getType() const;

	private:

		GLuint ID = 0u;
		ShaderType sType;

		void compile(char** shaderSource, unsigned int num);

	};

}

template <typename... Args>
void GL::ShaderLoader::init(const char* shaderSource, bool isFilePath, Args... args) {

	if (ID) throw Exception("Cannot call init on the same ShaderLoader twice.");
	
	CodeString code;
	code.init(shaderSource, isFilePath, args...);
	compile(code.getSourceCodeArray(), code.getSourceCodeArrayLength());

}

#endif