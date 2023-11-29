#ifndef PROGRAMUNIFORMS_HPP
#define PROGRAMUNIFORMS_HPP

#include <type_traits>

#include "./../util/GL-math.hpp"
#include "./../util/enums.hpp"
#include "./../util/util.hpp"

namespace GL {

	class ProgramUniforms : public _util {
	public:

		bool isInitialized() const;

		template <typename... Uniforms>
		void init(const char* name, UniformType dtype, unsigned int count, Uniforms... uniforms);

		unsigned int getUniformCount(const char* name, UniformType dtype) const;

		virtual void update() = 0;

		~ProgramUniforms();

	protected:

		char** uniforms = nullptr;
		UniformType* types = nullptr;
		unsigned int* offsets = nullptr;
		unsigned int* strides = nullptr;
		unsigned int* numElements = nullptr;
		GLint* IDs = nullptr;
		unsigned int numUniforms = 0u;
		unsigned int s, e;
		bool first = true;
		unsigned char* data;
		unsigned int dataSize = 0u;
		GLuint ID = 0u;
		bool std140, std430;

		template <typename T>
		bool typeIsValid(UniformType type) const;

		template <typename... Uniforms>
		void getNumUniforms(const char* name, UniformType dtype, unsigned int count, Uniforms... uniforms);
		void getNumUniforms(const char* name, UniformType dtype, unsigned int count);

		template <typename... Uniforms>
		void addUniform(unsigned int idx, unsigned int offset, const char* name, UniformType dtype, unsigned int count, Uniforms... uniforms);
		void addUniform(unsigned int idx, unsigned int offset, const char* name, UniformType dtype, unsigned int count);

		void addUniform_commonCode(unsigned int idx, unsigned int& offset, const char* name, UniformType dtype, unsigned int count);

	};

}

template <typename... Uniforms>
void GL::ProgramUniforms::init(const char* name, UniformType dtype, unsigned int count, Uniforms... uniforms) {
	
	if (this->uniforms) throw Exception("The init function for a ProgramUniforms object can only be called once.");

	getNumUniforms(name, dtype, count, uniforms...);

	this->uniforms = new char*[numUniforms];
	types = new UniformType[numUniforms];
	offsets = new unsigned int[numUniforms];
	strides = new unsigned int[numUniforms];
	numElements = new unsigned int[numUniforms];
	if (!std140 && !std430 && ID) IDs = new GLint[numUniforms];

	addUniform(0u, 0u, name, dtype, count, uniforms...);

	data = new unsigned char[dataSize];
	for (unsigned int i = 0u; i < dataSize; i++) data[i] = 0u;

}

template <typename T>
bool GL::ProgramUniforms::typeIsValid(UniformType type) const {

	return (
		(type == UniformType::INT && std::is_same<T, GLint>::value) ||
		(type == UniformType::UINT && std::is_same<T, GLuint>::value) ||
		(type == UniformType::FLOAT && std::is_same<T, GLfloat>::value) ||
		(type == UniformType::DOUBLE && std::is_same<T, GLdouble>::value) ||
		(type == UniformType::IVEC2 && std::is_same<T, ivec2>::value) ||
		(type == UniformType::UVEC2 && std::is_same<T, uvec2>::value) ||
		(type == UniformType::VEC2 && std::is_same<T, vec2>::value) ||
		(type == UniformType::DVEC2 && std::is_same<T, dvec2>::value) ||
		(type == UniformType::IVEC3 && std::is_same<T, ivec3>::value) ||
		(type == UniformType::UVEC3 && std::is_same<T, uvec3>::value) ||
		(type == UniformType::VEC3 && std::is_same<T, vec3>::value) ||
		(type == UniformType::DVEC3 && std::is_same<T, dvec3>::value) ||
		(type == UniformType::IVEC4 && std::is_same<T, ivec4>::value) ||
		(type == UniformType::UVEC4 && std::is_same<T, uvec4>::value) ||
		(type == UniformType::VEC4 && std::is_same<T, vec4>::value) ||
		(type == UniformType::DVEC4 && std::is_same<T, dvec4>::value) ||
		(type == UniformType::MAT2 && std::is_same<T, mat2>::value) ||
		(type == UniformType::DMAT2 && std::is_same<T, dmat2>::value) ||
		(type == UniformType::MAT3 && std::is_same<T, mat3>::value) ||
		(type == UniformType::DMAT3 && std::is_same<T, dmat3>::value) ||
		(type == UniformType::MAT4 && std::is_same<T, mat4>::value) ||
		(type == UniformType::DMAT4 && std::is_same<T, dmat4>::value) ||
		(type == UniformType::MAT2x3 && std::is_same<T, mat2x3>::value) ||
		(type == UniformType::DMAT2x3 && std::is_same<T, dmat2x3>::value) ||
		(type == UniformType::MAT2x4 && std::is_same<T, mat2x4>::value) ||
		(type == UniformType::DMAT2x4 && std::is_same<T, dmat2x4>::value) ||
		(type == UniformType::MAT3x2 && std::is_same<T, mat3x2>::value) ||
		(type == UniformType::DMAT3x2 && std::is_same<T, dmat3x2>::value) ||
		(type == UniformType::MAT3x4 && std::is_same<T, mat3x4>::value) ||
		(type == UniformType::DMAT3x4 && std::is_same<T, dmat3x4>::value) ||
		(type == UniformType::MAT4x2 && std::is_same<T, mat4x2>::value) ||
		(type == UniformType::DMAT4x2 && std::is_same<T, dmat4x2>::value) ||
		(type == UniformType::MAT4x3 && std::is_same<T, mat4x3>::value) ||
		(type == UniformType::DMAT4x3 && std::is_same<T, dmat4x3>::value)
	);

}

template <typename... Uniforms>
void GL::ProgramUniforms::getNumUniforms(const char* name, UniformType dtype, unsigned int count, Uniforms... uniforms) {

	if (count == 0u) throw Exception("Attempt to create a uniform entry \"" + std::string(name) + "\" with a count of zero.");

	numUniforms++;
	getNumUniforms(uniforms...);

}

template <typename... Uniforms>
void GL::ProgramUniforms::addUniform(unsigned int idx, unsigned int offset, const char* name, UniformType dtype, unsigned int count, Uniforms... uniforms) {

	addUniform_commonCode(idx, offset, name, dtype, count);
	addUniform(idx + 1u, offsets[idx] + strides[idx] * count, uniforms...);

}

#endif