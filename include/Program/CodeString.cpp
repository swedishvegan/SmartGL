
#include "./CodeString.hpp"

GL::CodeString::CodeString() { }

void GL::CodeString::init(char** shaderSource, unsigned int length) {
	
	if (isInit) throw Exception("Cannot call init on the same CodeString twice.");

	shaderSourceCode = shaderSource;
	numSources = length;

}

std::string GL::CodeString::getCodeString() const {

	std::string code;
	for (unsigned int i = 0u; i < numSources; i++) code += shaderSourceCode[i];
	return code;

}

char** GL::CodeString::getSourceCodeArray() const { return shaderSourceCode; }

unsigned int GL::CodeString::getSourceCodeArrayLength() const { return numSources; }

GL::CodeString::~CodeString() {

	if (shouldDelete) {

		for (unsigned int i = 0u; i < numSources; i++) if (shouldDelete[i]) delete[] shaderSourceCode[i];
		delete[] shaderSourceCode;
		delete[] shouldDelete;

	}

}

void GL::CodeString::getNumSources(const char*, bool) { numSources++; }

void GL::CodeString::addSources(unsigned int index, const char* shaderSource, bool isFilePath) {

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

