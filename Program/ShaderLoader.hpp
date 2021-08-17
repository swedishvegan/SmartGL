#ifndef SHADERLOADER_HPP
#define SHADERLOADER_HPP

#include <fstream>

#include "util/util.hpp"
#include "util/enums.hpp"
#include "util/GL-math.hpp"

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
		
		char** shaderSourceCode = nullptr;
		bool* shouldDelete = nullptr;
		unsigned int numSources;

		template <typename... Args>
		void getNumSources(const char*, bool, Args... args);

		void getNumSources(const char*, bool);

		template <typename... Args>
		void addSources(unsigned int index, const char* shaderSource, bool isFilePath, Args... args);

		void addSources(unsigned int index, const char* shaderSource, bool isFilePath);

		void compile(char** shaderSource, unsigned int num);

	};

}

GL::ShaderLoader::ShaderLoader(GL::ShaderType shaderType) : sType(shaderType) { }

GL::ShaderLoader::ShaderLoader(const ShaderLoader& copy) {

	if (copy.ID == 0u) throw Exception("Attempt to call the copy constructor on an uninitialized ShaderLoader.");

	ID = copy.ID;
	sType = copy.sType;

}

bool GL::ShaderLoader::isInitialized() const { return ID; }

template <typename... Args>
void GL::ShaderLoader::init(const char* shaderSource, bool isFilePath, Args... args) {

	if (ID) throw Exception("Cannot call init on the same ShaderLoader twice.");

	getNumSources(shaderSource, isFilePath, args...);

	shaderSourceCode = new char*[numSources];
	shouldDelete = new bool[numSources];

	addSources(0u, shaderSource, isFilePath, args...);
	compile(shaderSourceCode, numSources);

	for (unsigned int i = 0u; i < numSources; i++) if (shouldDelete[i]) delete[] shaderSourceCode[i];
	delete[] shaderSourceCode;

}

void GL::ShaderLoader::init(char** shaderSource, unsigned int length) { compile(shaderSource, length); }

GLuint GL::ShaderLoader::getID() const { return ID; }

GL::ShaderType GL::ShaderLoader::getType() const { 

	if (!isInitialized()) throw Exception("An uninitialized ShaderLoader has no type.");
	return sType;

}

template <typename... Args>
void GL::ShaderLoader::getNumSources(const char*, bool, Args... args) {
	
	numSources++;
	getNumSources(args...);

}

void GL::ShaderLoader::getNumSources(const char*, bool) { numSources++; }

template <typename... Args>
void GL::ShaderLoader::addSources(unsigned int index, const char* shaderSource, bool isFilePath, Args... args) {

	addSources(index, shaderSource, isFilePath);
	addSources(index + 1u, args...);

}

void GL::ShaderLoader::addSources(unsigned int index, const char* shaderSource, bool isFilePath) {

	if (isFilePath) {

		std::ifstream file(shaderSource, std::ios::binary);
		if (!file.is_open()) throw Exception("Failed to load shader code from file \"" + std::string(shaderSource) + ".\"");

		file.seekg(0, file.end);
		GLint fileLength = file.tellg();
		file.seekg(0, file.beg);
		fileLength -= file.tellg();
		if (fileLength < 1) throw Exception("Failed to load shader code from file \"" + std::string(shaderSource) + ".\"");

		shaderSourceCode[index] = new char[fileLength + 1];
		file.read(shaderSourceCode[index], fileLength);
		shaderSourceCode[index][fileLength] = '\0';

		shouldDelete[index] = true;

	}
	else {

		shaderSourceCode[index] = (char*)shaderSource;
		shouldDelete[index] = false;

	}

}

void GL::ShaderLoader::compile(char** shaderSource, unsigned int num) {
	
	static const GLenum shaderTypes[] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, GL_COMPUTE_SHADER };
	GLenum shaderType = shaderTypes[(int)sType];

	ID = glCreateShader(shaderType);
	glShaderSource(ID, num, shaderSource, nullptr);
	glCompileShader(ID);

	GLint compileStatus;
	glGetShaderiv(ID, GL_COMPILE_STATUS, &compileStatus);
	if (compileStatus == GL_FALSE) {

		GLint infoLogSize;
		glGetShaderiv(ID, GL_INFO_LOG_LENGTH, &infoLogSize);
		if (infoLogSize < 1) throw Exception("Shader failed to compile, info log unavailable.");
		else {

			char* infoLog = new char[infoLogSize];
			glGetShaderInfoLog(ID, infoLogSize, nullptr, infoLog);
			throw Exception("Shader failed to compile. Info log: " + std::string(infoLog));
			delete[] infoLog;

		}

	}
	
}

#endif