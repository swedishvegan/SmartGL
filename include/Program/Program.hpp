#ifndef PROGRAM_HPP
#define PROGRAM_HPP

#include <fstream>

#include "./ShaderLoader.hpp"

#define GL_LoadProgramFrom(programName, baseDirectory) \
GL::ShaderLoader programName ## _vs(GL::ShaderType::VERTEX); \
programName ## _vs.init( baseDirectory # programName ".vert", true); \
GL::ShaderLoader programName ## _fs(GL::ShaderType::FRAGMENT); \
programName ## _fs.init( baseDirectory # programName ".frag", true); \
GL::Program programName; \
programName.init(programName ## _vs, programName ## _fs);

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

template <typename... Args>
void GL::Program::init(ShaderLoader shader, Args... args) {

	if (!ID) ID = glCreateProgram();

	glAttachShader(ID, shader.getID());
	if (shader.getType() == ShaderType::COMPUTE) isComputeProgram = true;
	init(args...);

}

#endif