#ifndef UNIFORMTABLE_HPP
#define UNIFORMTABLE_HPP

#include "Program/Program.hpp"
#include "Uniform/ProgramUniforms.hpp"

namespace GL {

	class UniformTable : public ProgramUniforms {
	public:

		UniformTable(Program& program);

		template <typename T>
		T get(const char* name) const;

		template <typename T>
		void set(const char* name, T value);

		template <typename T>
		T getElement(const char* name, unsigned int index) const;

		template <typename T>
		void setElement(const char* name, unsigned int index, T value);

		void update();

	protected:

		unsigned int get_commonCode(const char* name) const;

		void updateSE(unsigned int idx);

	};

}

GL::UniformTable::UniformTable(Program& program) {
	
	ID = program.getID(); 
	std140 = false;

}

#define _GL_UniformTable_getSet_commonCode(getOrSet) \
if (!isInitialized()) throw Exception("Attempt to " # getOrSet " the uniform \"" + std::string(name) + "\" from an uninitialized uniform table."); \
\
unsigned int idx = get_commonCode(name); \
\
if (idx == numUniforms) throw Exception("Invalid uniform name \"" + std::string(name) + "\" passed to uniform table's " # getOrSet " function."); \
if (!typeIsValid<T>(types[idx])) throw Exception("Invalid data type passed to uniform table's " # getOrSet " function (uniform name passed was \"" + std::string(name) + "\").");

template <typename T>
T GL::UniformTable::get(const char* name) const {

	_GL_UniformTable_getSet_commonCode(get);
	
	return *(T*)(data + offsets[idx]);

}

template <typename T>
void GL::UniformTable::set(const char* name, T value) {

	_GL_UniformTable_getSet_commonCode(set);

	updateSE(idx);
	*(T*)(data + offsets[idx]) = value;

}

template <typename T>
T GL::UniformTable::getElement(const char* name, unsigned int index) const {

	_GL_UniformTable_getSet_commonCode(get);
	if (index >= numElements[idx]) throw Exception("Invalid uniform array index " + std::to_string(index) + " passed to uniform buffer table's get function (uniform name passed was \"" + std::string(name) + "\").");
	
	updateSE(idx);
	return *(T*)(data + offsets[idx] + index * strides[idx]);

}

template <typename T>
void GL::UniformTable::setElement(const char* name, unsigned int index, T value) {

	_GL_UniformTable_getSet_commonCode(set);
	if (index >= numElements[idx]) throw Exception("Invalid uniform array index " + std::to_string(index) + " passed to uniform buffer table's set function (uniform name passed was \"" + std::string(name) + "\").");

	updateSE(idx);
	*(T*)(data + offsets[idx] + index * strides[idx]) = value;

}

void GL::UniformTable::update() {

	if (!isInitialized()) throw Exception("Attempt to call the update function in an uninitialized uniform table.");
	if (first) return;

	glUseProgram(ID);

	for (unsigned int idx = s; idx <= e; idx++) {

		UniformType type = types[idx];
		GLint loc = IDs[idx];
		unsigned int num = numElements[idx];
		unsigned char* dataStart = data + offsets[idx];
		
		if (type == UniformType::INT) glUniform1iv(loc, num, (GLint*)dataStart);
		else if (type == UniformType::UINT) glUniform1uiv(loc, num, (GLuint*)dataStart);
		else if (type == UniformType::FLOAT) glUniform1fv(loc, num, (GLfloat*)dataStart);
		else if (type == UniformType::DOUBLE) glUniform1dv(loc, num, (GLdouble*)dataStart);
		else if (type == UniformType::IVEC2) glUniform2iv(loc, num, (GLint*)dataStart);
		else if (type == UniformType::UVEC2) glUniform2uiv(loc, num, (GLuint*)dataStart);
		else if (type == UniformType::VEC2) glUniform2fv(loc, num, (GLfloat*)dataStart);
		else if (type == UniformType::DVEC2) glUniform2dv(loc, num, (GLdouble*)dataStart);
		else if (type == UniformType::IVEC3) glUniform3iv(loc, num, (GLint*)dataStart);
		else if (type == UniformType::UVEC3) glUniform3uiv(loc, num, (GLuint*)dataStart);
		else if (type == UniformType::VEC3) glUniform3fv(loc, num, (GLfloat*)dataStart);
		else if (type == UniformType::DVEC3) glUniform3dv(loc, num, (GLdouble*)dataStart);
		else if (type == UniformType::IVEC4) glUniform4iv(loc, num, (GLint*)dataStart);
		else if (type == UniformType::UVEC4) glUniform4uiv(loc, num, (GLuint*)dataStart);
		else if (type == UniformType::VEC4) glUniform4fv(loc, num, (GLfloat*)dataStart);
		else if (type == UniformType::DVEC4) glUniform4dv(loc, num, (GLdouble*)dataStart);
		else if (type == UniformType::MAT2) glUniformMatrix2fv(loc, 1, GL_FALSE, (GLfloat*)dataStart);
		else if (type == UniformType::DMAT2) glUniformMatrix2dv(loc, 1, GL_FALSE, (GLdouble*)dataStart);
		else if (type == UniformType::MAT3) glUniformMatrix3fv(loc, 1, GL_FALSE, (GLfloat*)dataStart);
		else if (type == UniformType::DMAT3) glUniformMatrix3dv(loc, 1, GL_FALSE, (GLdouble*)dataStart);
		else if (type == UniformType::MAT4) glUniformMatrix4fv(loc, 1, GL_FALSE, (GLfloat*)dataStart);
		else if (type == UniformType::DMAT4) glUniformMatrix4dv(loc, 1, GL_FALSE, (GLdouble*)dataStart);
		else if (type == UniformType::MAT2x3) glUniformMatrix3x2fv(loc, 1, GL_FALSE, (GLfloat*)dataStart);
		else if (type == UniformType::DMAT2x3) glUniformMatrix3x2dv(loc, 1, GL_FALSE, (GLdouble*)dataStart);
		else if (type == UniformType::MAT2x4) glUniformMatrix4x2fv(loc, 1, GL_FALSE, (GLfloat*)dataStart);
		else if (type == UniformType::DMAT2x4) glUniformMatrix4x2dv(loc, 1, GL_FALSE, (GLdouble*)dataStart);
		else if (type == UniformType::MAT3x2) glUniformMatrix2x3fv(loc, 1, GL_FALSE, (GLfloat*)dataStart);
		else if (type == UniformType::DMAT3x2) glUniformMatrix2x3dv(loc, 1, GL_FALSE, (GLdouble*)dataStart);
		else if (type == UniformType::MAT3x4) glUniformMatrix4x3fv(loc, 1, GL_FALSE, (GLfloat*)dataStart);
		else if (type == UniformType::DMAT3x4) glUniformMatrix4x3dv(loc, 1, GL_FALSE, (GLdouble*)dataStart);
		else if (type == UniformType::MAT4x2) glUniformMatrix2x4fv(loc, 1, GL_FALSE, (GLfloat*)dataStart);
		else if (type == UniformType::DMAT4x2) glUniformMatrix2x4dv(loc, 1, GL_FALSE, (GLdouble*)dataStart);
		else if (type == UniformType::MAT4x3) glUniformMatrix3x4fv(loc, 1, GL_FALSE, (GLfloat*)dataStart);
		else glUniformMatrix3x4dv(loc, 1, GL_FALSE, (GLdouble*)dataStart);

	}

	first = true;

}

unsigned int GL::UniformTable::get_commonCode(const char* name) const {

	unsigned int idx = numUniforms;
	for (unsigned int i = 0u; i < numUniforms; i++)

		if (!strcmp(name, uniforms[i])) {

			idx = i;
			break;

		}

	return idx;

}

 void GL::UniformTable::updateSE(unsigned int idx) {

	if (first) {

		s = idx; e = idx;
		first = false;

	}
	else {

		if (idx < s) s = idx;
		if (idx > e) e = idx;

	}

}

#endif