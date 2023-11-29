#ifndef CODESTRING_HPP
#define CODESTRING_HPP

#include <string>
#include <fstream>
#include "./../util/util.hpp"

namespace GL {

	class CodeString : public _util {
	public:

		CodeString();

		template <typename... Args>
		void init(const char* shaderSource, bool isFilePath, Args... args);

		void init(char** shaderSource, unsigned int length);

		std::string getCodeString() const;

		char** getSourceCodeArray() const;

		unsigned int getSourceCodeArrayLength() const;

		~CodeString();

	private:

		bool isInit = false;

		char** shaderSourceCode = nullptr;
		bool* shouldDelete = nullptr;
		unsigned int numSources = 0u;

		template <typename... Args>
		void getNumSources(const char*, bool, Args... args);

		void getNumSources(const char*, bool);

		template <typename... Args>
		void addSources(unsigned int index, const char* shaderSource, bool isFilePath, Args... args);

		void addSources(unsigned int index, const char* shaderSource, bool isFilePath);

	};

}

template <typename... Args>
void GL::CodeString::init(const char* shaderSource, bool isFilePath, Args... args) {

	if (isInit) throw Exception("Cannot call init on the same CodeString twice.");
	
	getNumSources(shaderSource, isFilePath, args...);

	shaderSourceCode = new char* [numSources];
	shouldDelete = new bool[numSources];

	addSources(0u, shaderSource, isFilePath, args...);

}

template <typename... Args>
void GL::CodeString::getNumSources(const char*, bool, Args... args) {

	numSources++;
	getNumSources(args...);

}

template <typename... Args>
void GL::CodeString::addSources(unsigned int index, const char* shaderSource, bool isFilePath, Args... args) {

	addSources(index, shaderSource, isFilePath);
	addSources(index + 1u, args...);

}

#endif