#ifndef BUFFERTABLE_HPP
#define BUFFERTABLE_HPP

#include <cstring>

#include "./ProgramUniforms.hpp"

namespace GL {

	class BufferTable : public ProgramUniforms {
	public:

		void bind() const;

		void bindToIndex(unsigned int idx) const;

		template <typename T>
		T get(const char* name) const;

		template <typename T>
		void set(const char* name, T value);

		template <typename T>
		T getElement(const char* name, unsigned int index) const;

		template <typename T>
		void setElement(const char* name, unsigned int index, T value);

		void update();

		~BufferTable();

	protected:

		unsigned int bindingPoint;
		GLenum bufferType;
		bool bufferNeedsAlloc = true;

		BufferTable(unsigned int bindingPoint, GLenum bufferType);

		unsigned int getset_getIndex(const char* name) const;

		template <typename T>
		T get_commonCode(unsigned int idx, unsigned int index) const;

		template <typename T>
		void set_commonCode(unsigned int idx, unsigned int index, T value);

		unsigned int getMatrixStride(UniformType type) const;

	};

}

template <typename T>
T GL::BufferTable::get(const char* name) const {

	if (!isInitialized()) throw Exception("Attempt to get the variable \"" + std::string(name) + "\" from an uninitialized buffer table.");

	unsigned int idx = getset_getIndex(name);

	if (idx == numUniforms) throw Exception("Invalid variable name \"" + std::string(name) + "\" passed to buffer table's get function.");
	if (!typeIsValid<T>(types[idx])) throw Exception("Invalid data type passed to buffer table's get function (variable name passed was \"" + std::string(name) + "\").");

	return get_commonCode<T>(idx, 0u);

}

template <typename T>
void GL::BufferTable::set(const char* name, T value) {

	if (!isInitialized()) throw Exception("Attempt to set the variable \"" + std::string(name) + "\" in an uninitialized buffer table.");

	unsigned int idx = getset_getIndex(name);

	if (idx == numUniforms) throw Exception("Invalid variable name \"" + std::string(name) + "\" passed to buffer table's set function.");
	if (!typeIsValid<T>(types[idx])) throw Exception("Invalid data type passed to buffer table's set function (variable name passed was \"" + std::string(name) + "\").");

	set_commonCode<T>(idx, 0u, value);

}

template <typename T>
T GL::BufferTable::getElement(const char* name, unsigned int index) const {

	if (!isInitialized()) throw Exception("Attempt to get the variable \"" + std::string(name) + "\" element " + std::to_string(index) + " from an uninitialized buffer table.");

	unsigned int idx = getset_getIndex(name);

	if (idx == numUniforms) throw Exception("Invalid variable name \"" + std::string(name) + "\" (element " + std::to_string(index) + ") passed to buffer table's get function.");
	if (!typeIsValid<T>(types[idx])) throw Exception("Invalid data type passed to buffer table's get function (variable name passed was \"" + std::string(name) + "\").");
	if (index >= numElements[idx]) throw Exception("Invalid array index " + std::to_string(index) + " passed to buffer table's get function (variable name passed was \"" + std::string(name) + "\").");

	return get_commonCode<T>(idx, index);

}

template <typename T>
void GL::BufferTable::setElement(const char* name, unsigned int index, T value) {

	if (!isInitialized()) throw Exception("Attempt to set the variable \"" + std::string(name) + "\" element " + std::to_string(index) + " from an uninitialized buffer table.");

	unsigned int idx = getset_getIndex(name);

	if (idx == numUniforms) throw Exception("Invalid variable name \"" + std::string(name) + "\" (element " + std::to_string(index) + ") passed to buffer table's set function.");
	if (!typeIsValid<T>(types[idx])) throw Exception("Invalid data type passed to buffer table's set function (variable name passed was \"" + std::string(name) + "\").");
	if (index >= numElements[idx]) throw Exception("Invalid array index (" + std::to_string(index) + ") passed to buffer table's set function (variable name passed was \"" + std::string(name) + "\").");

	set_commonCode<T>(idx, index, value);

}

template <typename T>
T GL::BufferTable::get_commonCode(unsigned int idx, unsigned int index) const {

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
void GL::BufferTable::set_commonCode(unsigned int idx, unsigned int index, T value) {

	unsigned int potentialS = offsets[idx] + strides[idx] * index;
	unsigned int potentialE = potentialS + strides[idx] - 1u;

	unsigned char* dataStart = data + potentialS;
	unsigned int matStride = getMatrixStride(types[idx]);

	unsigned char* src = (unsigned char*)(&value);

	if (matStride) {

		unsigned int arrIdx = (unsigned int)types[idx] - 16u;

		static const unsigned int matNumCols[] = {
			2u, 2u, 3u, 3u, 4u, 4u, 3u, 3u, 4u, 4u, 2u, 2u, 4u, 4u, 2u, 2u, 3u, 3u
		};

		static const unsigned int matColSizes[] = {
			8u, 16u, 12u, 24u, 16u, 32u, 8u, 16u, 8u, 16u, 12u, 24u, 12u, 24u, 16u, 32u, 16u, 32u
		};

		for (unsigned int i = 0u; i < matNumCols[arrIdx]; i++)

			std::memcpy(
				dataStart + i * matStride,
				src + i * matColSizes[arrIdx],
				matColSizes[arrIdx]
			);

	}
	else std::memcpy(
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

#endif