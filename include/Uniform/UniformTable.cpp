
#include "./UniformTable.hpp"

GL::UniformTable::UniformTable() {
	
	isOwned = false;
	std140 = false;

}

GL::UniformTable::UniformTable(Program& program) {
	
	ID = program.getID();
	isOwned = true;
	std140 = false;

}

void GL::UniformTable::update() { 
	
	if (!isOwned) throw Exception("Update function with no parameters cannot be called on a uniform table with no owner program.");
	update_commonCode(ID); 

}

void GL::UniformTable::update(GL::Program& program) { 
	
	if (isOwned) throw Exception("Update function with a parameter cannot be called on a uniform table with an owner program.");
	update_commonCode(program.getID());

}

void GL::UniformTable::update_commonCode(unsigned int progID) {

	if (!isInitialized()) throw Exception("Attempt to call the update function in an uninitialized uniform table.");
	if (first) return;

	glUseProgram(progID);

	for (unsigned int idx = s; idx <= e; idx++) {

		UniformType type = types[idx];
		GLint loc = (IDs) ? IDs[idx] : glGetUniformLocation(progID, uniforms[idx]);
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
		else if (type == UniformType::MAT2) glUniformMatrix2fv(loc, num, GL_FALSE, (GLfloat*)dataStart);
		else if (type == UniformType::DMAT2) glUniformMatrix2dv(loc, num, GL_FALSE, (GLdouble*)dataStart);
		else if (type == UniformType::MAT3) glUniformMatrix3fv(loc, num, GL_FALSE, (GLfloat*)dataStart);
		else if (type == UniformType::DMAT3) glUniformMatrix3dv(loc, num, GL_FALSE, (GLdouble*)dataStart);
		else if (type == UniformType::MAT4) glUniformMatrix4fv(loc, num, GL_FALSE, (GLfloat*)dataStart);
		else if (type == UniformType::DMAT4) glUniformMatrix4dv(loc, num, GL_FALSE, (GLdouble*)dataStart);
		else if (type == UniformType::MAT2x3) glUniformMatrix3x2fv(loc, num, GL_FALSE, (GLfloat*)dataStart);
		else if (type == UniformType::DMAT2x3) glUniformMatrix3x2dv(loc, num, GL_FALSE, (GLdouble*)dataStart);
		else if (type == UniformType::MAT2x4) glUniformMatrix4x2fv(loc, num, GL_FALSE, (GLfloat*)dataStart);
		else if (type == UniformType::DMAT2x4) glUniformMatrix4x2dv(loc, num, GL_FALSE, (GLdouble*)dataStart);
		else if (type == UniformType::MAT3x2) glUniformMatrix2x3fv(loc, num, GL_FALSE, (GLfloat*)dataStart);
		else if (type == UniformType::DMAT3x2) glUniformMatrix2x3dv(loc, num, GL_FALSE, (GLdouble*)dataStart);
		else if (type == UniformType::MAT3x4) glUniformMatrix4x3fv(loc, num, GL_FALSE, (GLfloat*)dataStart);
		else if (type == UniformType::DMAT3x4) glUniformMatrix4x3dv(loc, num, GL_FALSE, (GLdouble*)dataStart);
		else if (type == UniformType::MAT4x2) glUniformMatrix2x4fv(loc, num, GL_FALSE, (GLfloat*)dataStart);
		else if (type == UniformType::DMAT4x2) glUniformMatrix2x4dv(loc, num, GL_FALSE, (GLdouble*)dataStart);
		else if (type == UniformType::MAT4x3) glUniformMatrix3x4fv(loc, num, GL_FALSE, (GLfloat*)dataStart);
		else glUniformMatrix3x4dv(loc, num, GL_FALSE, (GLdouble*)dataStart);

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