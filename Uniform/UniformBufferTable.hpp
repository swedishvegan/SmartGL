#ifndef UNIFORMBUFFERTABLE_HPP
#define UNIFORMBUFFERTABLE_HPP

#include "Uniform/ProgramUniforms.hpp"

namespace GL {

	class UniformBufferTable : public ProgramUniforms {
	public:

		UniformBufferTable(unsigned int bindingPoint);

		void bind() const;

		template <typename T>
		T get(const char* name) const;

		template <typename T>
		void set(const char* name, T value);

		template <typename T>
		T getElement(const char* name, unsigned int index) const;

		template <typename T>
		void setElement(const char* name, unsigned int index, T value);

		void update();

		~UniformBufferTable();

	private:

		unsigned int bindingPoint;

		unsigned int getset_getIndex(const char* name) const;

		template <typename T>
		T get_commonCode(unsigned int idx, unsigned int index) const;

		template <typename T>
		void set_commonCode(unsigned int idx, unsigned int index, T value);

		static unsigned int getMatrixStride(UniformType type);

	};

}

GL::UniformBufferTable::UniformBufferTable(unsigned int bindingPoint) : bindingPoint(bindingPoint) {

	if (bindingPoint >= _util::maxUniformBufferBindings) throw Exception("Uniform buffer binding point supplied was " + std::to_string(bindingPoint) + ", but the maximum allowed binding point is " + std::to_string(_util::maxUniformBufferBindings) + ".");

	glGenBuffers(1, &ID);
	bind();

	std140 = true;

}

void GL::UniformBufferTable::bind() const {

	glBindBuffer(GL_UNIFORM_BUFFER, ID);
	glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, ID);

}

template <typename T>
T GL::UniformBufferTable::get(const char* name) const {

	if (!isInitialized()) throw Exception("Attempt to get the uniform \"" + std::string(name) + "\" from an uninitialized uniform buffer table.");

	unsigned int idx = getset_getIndex(name);

	if (idx == numUniforms) throw Exception("Invalid uniform name \"" + std::string(name) + "\" passed to uniform buffer table's get function.");
	if (!typeIsValid<T>(types[idx])) throw Exception("Invalid data type passed to uniform buffer table's get function (uniform name passed was \"" + std::string(name) + "\").");

	return get_commonCode<T>(idx, 0u);

}

template <typename T>
void GL::UniformBufferTable::set(const char* name, T value) {

	if (!isInitialized()) throw Exception("Attempt to set the uniform \"" + std::string(name) + "\" in an uninitialized uniform buffer table.");
	
	unsigned int idx = getset_getIndex(name);

	if (idx == numUniforms) throw Exception("Invalid uniform name \"" + std::string(name) + "\" passed to uniform buffer table's set function.");
	if (!typeIsValid<T>(types[idx])) throw Exception("Invalid data type passed to uniform buffer table's set function (uniform name passed was \"" + std::string(name) + "\").");
	
	set_commonCode<T>(idx, 0u, value);

}

template <typename T>
T GL::UniformBufferTable::getElement(const char* name, unsigned int index) const {

	if (!isInitialized()) throw Exception("Attempt to get the uniform \"" + std::string(name) + "\" element " + std::to_string(index) + " from an uninitialized uniform buffer table.");

	unsigned int idx = getset_getIndex(name);

	if (idx == numUniforms) throw Exception("Invalid uniform name \"" + std::string(name) + "\" (element "  + std::to_string(index) + ") passed to uniform buffer table's get function.");
	if (!typeIsValid<T>(types[idx])) throw Exception("Invalid data type passed to uniform buffer table's get function (uniform name passed was \"" + std::string(name) + "\").");
	if (index >= numElements[idx]) throw Exception("Invalid uniform array index " + std::to_string(index) + " passed to uniform buffer table's get function (uniform name passed was \"" + std::string(name) + "\").");
	
	return get_commonCode<T>(idx, index);

}

template <typename T>
void GL::UniformBufferTable::setElement(const char* name, unsigned int index, T value) {

	if (!isInitialized()) throw Exception("Attempt to set the uniform \"" + std::string(name) + "\" element " + std::to_string(index) + " from an uninitialized uniform buffer table.");

	unsigned int idx = getset_getIndex(name);

	if (idx == numUniforms) throw Exception("Invalid uniform name \"" + std::string(name) + "\" (element " + std::to_string(index) + ") passed to uniform buffer table's set function.");
	if (!typeIsValid<T>(types[idx])) throw Exception("Invalid data type passed to uniform buffer table's set function (uniform name passed was \"" + std::string(name) + "\").");
	if (index >= numElements[idx]) throw Exception("Invalid uniform array index (" + std::to_string(index) + ") passed to uniform buffer table's set function (uniform name passed was \"" + std::string(name) + "\").");

	set_commonCode<T>(idx, index, value);

}

void GL::UniformBufferTable::update() {

	if (!isInitialized()) throw Exception("Attempt to call the update function in an uninitialized uniform buffer table.");
	if (first) return;

	bind();
	glBufferSubData(GL_UNIFORM_BUFFER, s, 1 + e - s, data + s);
	first = true;

}

GL::UniformBufferTable::~UniformBufferTable() { if (ID) glDeleteBuffers(1, &ID); }

unsigned int GL::UniformBufferTable::getset_getIndex(const char* name) const {

	unsigned int idx = numUniforms;
	for (unsigned int i = 0u; i < numUniforms; i++)

		if (!strcmp(name, uniforms[i])) {

			idx = i;
			break;

		}

	return idx;

}

template <typename T>
T GL::UniformBufferTable::get_commonCode(unsigned int idx, unsigned int index) const {

	unsigned char* dataStart = data + offsets[idx] + strides[idx] * index;
	unsigned int matStride = getMatrixStride(types[idx]);

	if (matStride) {

		if (std::is_same<T, mat2>::value) return upscale<T>(mat2(
			*(vec2*)(dataStart),
			*(vec2*)(dataStart + matStride)
		));
		else if (std::is_same<T, dmat2>::value) return upscale<T>(dmat2(
			*(dvec2*)(dataStart),
			*(dvec2*)(dataStart + matStride)
		));

		else if (std::is_same<T, mat3>::value) return upscale<T>(mat3(
			*(vec3*)(dataStart),
			*(vec3*)(dataStart + matStride),
			*(vec3*)(dataStart + 2u * matStride)
		));
		else if (std::is_same<T, dmat3>::value) return upscale<T>(dmat3(
			*(dvec3*)(dataStart),
			*(dvec3*)(dataStart + matStride),
			*(dvec3*)(dataStart + 2u * matStride)
		));

		else if (std::is_same<T, mat4>::value) return upscale<T>(mat4(
			*(vec4*)(dataStart),
			*(vec4*)(dataStart + matStride),
			*(vec4*)(dataStart + 2u * matStride),
			*(vec4*)(dataStart + 3u * matStride)
		));
		else if (std::is_same<T, dmat4>::value) return upscale<T>(dmat4(
			*(dvec4*)(dataStart),
			*(dvec4*)(dataStart + matStride),
			*(dvec4*)(dataStart + 2u * matStride),
			*(dvec4*)(dataStart + 3u * matStride)
		));

		else if (std::is_same<T, mat2x3>::value) return upscale<T>(mat2x3(
			*(vec2*)(dataStart),
			*(vec2*)(dataStart + matStride),
			*(vec2*)(dataStart + 2u * matStride)
		));
		else if (std::is_same<T, dmat2x3>::value) return upscale<T>(dmat2x3(
			*(dvec2*)(dataStart),
			*(dvec2*)(dataStart + matStride),
			*(dvec2*)(dataStart + 2u * matStride)
		));

		else if (std::is_same<T, mat2x4>::value) return upscale<T>(mat2x4(
			*(vec2*)(dataStart),
			*(vec2*)(dataStart + matStride),
			*(vec2*)(dataStart + 2u * matStride),
			*(vec2*)(dataStart + 3u * matStride)
		));
		else if (std::is_same<T, dmat2x4>::value) return upscale<T>(dmat2x4(
			*(dvec2*)(dataStart),
			*(dvec2*)(dataStart + matStride),
			*(dvec2*)(dataStart + 2u * matStride),
			*(dvec2*)(dataStart + 3u * matStride)
		));

		else if (std::is_same<T, mat3x2>::value) return upscale<T>(mat3x2(
			*(vec3*)(dataStart),
			*(vec3*)(dataStart + matStride)
		));
		else if (std::is_same<T, dmat3x2>::value) return upscale<T>(dmat3x2(
			*(dvec3*)(dataStart),
			*(dvec3*)(dataStart + matStride)
		));

		else if (std::is_same<T, mat3x4>::value) return upscale<T>(mat3x4(
			*(vec3*)(dataStart),
			*(vec3*)(dataStart + matStride),
			*(vec3*)(dataStart + 2u * matStride),
			*(vec3*)(dataStart + 3u * matStride)
		));
		else if (std::is_same<T, dmat3x4>::value) return upscale<T>(dmat3x4(
			*(dvec3*)(dataStart),
			*(dvec3*)(dataStart + matStride),
			*(dvec3*)(dataStart + 2u * matStride),
			*(dvec3*)(dataStart + 3u * matStride)
		));

		else if (std::is_same<T, mat4x2>::value) return upscale<T>(mat4x2(
			*(vec4*)(dataStart),
			*(vec4*)(dataStart + matStride)
		));
		else if (std::is_same<T, dmat4x2>::value) upscale<T>(dmat4x2(
			*(dvec4*)(dataStart),
			*(dvec4*)(dataStart + matStride)
		));

		else if (std::is_same<T, mat4x3>::value) return upscale<T>(mat4x3(
			*(vec4*)(dataStart),
			*(vec4*)(dataStart + matStride),
			*(vec4*)(dataStart + 2u * matStride)
		));
		else return upscale<T>(dmat4x3(
			*(dvec4*)(dataStart),
			*(dvec4*)(dataStart + matStride),
			*(dvec4*)(dataStart + 2u * matStride)
		));

	}
	else return *(T*)(dataStart);

}

template <typename T>
void GL::UniformBufferTable::set_commonCode(unsigned int idx, unsigned int index, T value) {

	unsigned int potentialS = offsets[idx] + strides[idx] * index;
	unsigned int potentialE = potentialS + strides[idx] - 1u;

	unsigned char* dataStart = data + potentialS;
	unsigned int matStride = getMatrixStride(types[idx]);

	unsigned char* src = (unsigned char*)(&value);

	if (matStride) {
		
		unsigned int arrIdx = (unsigned int)types[idx] - 16u;

		static const unsigned int _matNumCols[] = {
			2u, 2u, 3u, 3u, 4u, 4u, 3u, 3u, 4u, 4u, 2u, 2u, 4u, 4u, 2u, 2u, 3u, 3u
		};

		static const unsigned int _matColSizes[] = {
			8u, 16u, 12u, 24u, 16u, 32u, 8u, 16u, 8u, 16u, 12u, 24u, 12u, 24u, 16u, 32u, 16u, 32u
		};

		for (unsigned int i = 0u; i < _matNumCols[arrIdx]; i++)

			memcpy(
				dataStart + i * matStride,
				src + i * _matColSizes[arrIdx],
				_matColSizes[arrIdx]
			);

	}
	else memcpy(
		dataStart,
		src,
		sizeof(T)
	);

	if (first) {

		s = potentialS; e = potentialE;
		first = false;

	}
	else {

		if (potentialS < s) s = potentialS;
		if (potentialE > e) e = potentialE;

	}

}

unsigned int GL::UniformBufferTable::getMatrixStride(UniformType type) {

	if ((int)type < 16) return 0u;
	return _util::std140arrayAlignments[(int)type];

}

#endif