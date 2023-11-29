
#include "./ShaderLoader.hpp"

GL::ShaderLoader::ShaderLoader(GL::ShaderType shaderType) : sType(shaderType) { }

GL::ShaderLoader::ShaderLoader(const ShaderLoader& copy) {

	if (copy.ID == 0u) throw Exception("Attempt to call the copy constructor on an uninitialized ShaderLoader.");

	ID = copy.ID;
	sType = copy.sType;

}

bool GL::ShaderLoader::isInitialized() const { return ID; }

void GL::ShaderLoader::init(char** shaderSource, unsigned int length) { 

	CodeString code;
	code.init(shaderSource, length);
	compile(shaderSource, length);

}

GLuint GL::ShaderLoader::getID() const { return ID; }

GL::ShaderType GL::ShaderLoader::getType() const { return sType; }

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