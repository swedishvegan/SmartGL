#ifndef PROGRAM_HPP
#define PROGRAM_HPP

#include <fstream>

#include "Program/ShaderLoader.hpp"

#define GL_LoadProgramFrom(programName, baseDirectory) \
GL::ShaderLoader programName ## _vs(GL::ShaderType::VERTEX); \
programName ## _vs.init( # baseDirectory # programName ".vert", true); \
GL::ShaderLoader programName ## _fs(GL::ShaderType::FRAGMENT); \
programName ## _fs.init( # baseDirectory # programName ".frag", true); \
GL::Program programName; \
programName ## .init(programName ## _vs, programName ## _fs);

#define GL_LoadProgram(programName) GL_LoadProgramFrom(programName,)

namespace GL {

	class Program : public _util {
	public:

		Program();

		bool isInitialized() const;

		template <typename... Args>
		void init(ShaderLoader shader, Args... args);

		void init(ShaderLoader shader);

		GLuint getID() const;

		void use() const;

		GLint getUniformLocation(const char* name) const;

		void dispatchCompute(uvec3 numWorkGroups, bool memoryBarrier = true);

		static uvec3 maxWorkGroups();

		static uvec3 maxWorkGroupSize();

		static unsigned int maxWorkGroupInvocations();

		~Program();

	private:

		GLuint ID = 0u;
		bool isComputeProgram;
		bool isInit = false;

	};

}

GL::Program::Program() : isComputeProgram(false) { }

bool GL::Program::isInitialized() const { return isInit; }

template <typename... Args>
void GL::Program::init(ShaderLoader shader, Args... args) {

	if (!ID) ID = glCreateProgram();

	glAttachShader(ID, shader.getID());
	if (shader.getType() == ShaderType::COMPUTE) isComputeProgram = true;
	init(args...);

}

void GL::Program::init(ShaderLoader shader) {

	if (!ID) ID = glCreateProgram();

	glAttachShader(ID, shader.getID());
	if (shader.getType() == ShaderType::COMPUTE) isComputeProgram = true;
	glLinkProgram(ID);

	GLint success;
	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (success == GL_FALSE) {

		GLint infoLogSize;
		glGetProgramiv(ID, GL_INFO_LOG_LENGTH, &infoLogSize);
		if (infoLogSize < 1) throw Exception("Program failed to link, info log unavailable.");
		else {

			char* infoLog = new char[infoLogSize];
			glGetProgramInfoLog(ID, infoLogSize, nullptr, infoLog);
			throw Exception("Program failed to link. Info log: " + std::string(infoLog));
			delete[] infoLog;

		}

	}

	isInit = true;

}

GLuint GL::Program::getID() const { return ID; }

void GL::Program::use() const { 

	if (!isInitialized()) throw Exception("Cannot use an uninitialized program.");
	glUseProgram(ID);

}

GLint GL::Program::getUniformLocation(const char* name) const {
	
	if (!isInitialized()) throw Exception("Cannot get uniform location \"" + std::string(name) + "\" from an uninitialized program.");
	return glGetUniformLocation(ID, name);

}

void GL::Program::dispatchCompute(uvec3 numWorkGroups, bool memoryBarrier) {

	if (!isComputeProgram) throw Exception("Attempt to call dispatchCompute (" + std::to_string(numWorkGroups.x) + "x" + std::to_string(numWorkGroups.y) + "x" + std::to_string(numWorkGroups.z) + " work groups and " + std::string((memoryBarrier) ? " " : "no ") + "memory barrier) on program with no compute shaders.");
	if (!isInitialized()) throw Exception("Cannot call dispatchCompute on an uninitialized program.");

	glUseProgram(ID);
	glDispatchCompute(numWorkGroups.x, numWorkGroups.y, numWorkGroups.z);
	if (memoryBarrier) glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

}

GL::uvec3 GL::Program::maxWorkGroups() { return uvec3(_util::maxComputeWorkGroupCount[0], _util::maxComputeWorkGroupCount[1], _util::maxComputeWorkGroupCount[2]); }

GL::uvec3 GL::Program::maxWorkGroupSize() { return uvec3(_util::maxComputeWorkGroupSize[0], _util::maxComputeWorkGroupSize[1], _util::maxComputeWorkGroupSize[2]); }

unsigned int GL::Program::maxWorkGroupInvocations() { return (unsigned int)_util::maxComputeWorkGroupInvocations; }

GL::Program::~Program() { if (ID) glDeleteProgram(ID); }

#endif