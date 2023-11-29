
#include <cstring>

#include "./ProgramUniforms.hpp"

bool GL::ProgramUniforms::isInitialized() const { return uniforms; }

unsigned int GL::ProgramUniforms::getUniformCount(const char* name, GL::UniformType dtype) const {

	if (uniforms == nullptr) throw Exception("Attempt to call getUniformCount() on an uninitialized ProgramUniforms object.");

	for (unsigned int i = 0u; i < numUniforms; i++) if (std::strcmp(name, uniforms[i]) == 0) if (dtype == types[i]) return numElements[i];
	return 0u;

}

GL::ProgramUniforms::~ProgramUniforms() {

	if (uniforms) {

		delete[] uniforms;
		delete[] types;
		delete[] offsets;
		delete[] strides;
		delete[] numElements;
		if (IDs) delete[] IDs;
		if (data) delete[] data;

	}

}

void GL::ProgramUniforms::getNumUniforms(const char* name, UniformType dtype, unsigned int count) { 
	
	if (count == 0u) throw Exception("Attempt to create a uniform entry \"" + std::string(name) + "\" with a count of zero.");
	numUniforms++; 

}

void GL::ProgramUniforms::addUniform(unsigned int idx, unsigned int offset, const char* name, UniformType dtype, unsigned int count) {

	addUniform_commonCode(idx, offset, name, dtype, count);
	dataSize = offset + strides[idx] * count;

}

void GL::ProgramUniforms::addUniform_commonCode(unsigned int idx, unsigned int& offset, const char* name, UniformType dtype, unsigned int count) {

	static const unsigned int uniformSizes[] = {

		4u, 4u, 4u, 8u, 8u, 8u, 8u, 16u, 12u, 12u, 12u, 24u, 16u, 16u, 16u, 32u,
		16u, 32u, 36u, 72u, 64u, 128u, 24u, 48u, 32u, 64u, 24u, 48u, 48u, 96u, 32u, 64u, 48u, 96u

	};

	static const unsigned int std140sizes[] = {

		4u, 4u, 4u, 8u, 8u, 8u, 8u, 16u, 16u, 16u, 16u, 32u, 16u, 16u, 16u, 32u,
		32u, 32u, 48u, 96u, 64u, 128u, 48u, 48u, 64u, 64u, 32u, 64u, 64u, 128u, 32u, 64u, 48u, 96u

	};

	static const unsigned int std430sizes[] = {

		4u, 4u, 4u, 8u, 8u, 8u, 8u, 16u, 16u, 16u, 16u, 32u, 16u, 16u, 16u, 32u,
		16u, 32u, 48u, 96u, 64u, 128u, 24u, 48u, 32u, 64u, 32u, 64u, 64u, 128u, 32u, 64u, 48u, 96u

	};

	static const unsigned int std140alignments[] = {

		4u, 4u, 4u, 8u, 8u, 8u, 8u, 16u, 16u, 16u, 16u, 32u, 16u, 16u, 16u, 32u,
		16u, 16u, 16u, 32u, 16u, 32u, 16u, 16u, 16u, 16u, 16u, 32u, 16u, 32u, 16u, 32u, 16u, 32u

	};

	static const unsigned int std430alignments[] = {

		4u, 4u, 4u, 8u, 8u, 8u, 8u, 16u, 16u, 16u, 16u, 32u, 16u, 16u, 16u, 32u,
		8u, 16u, 16u, 32u, 16u, 32u, 8u, 16u, 8u, 16u, 16u, 32u, 16u, 32u, 16u, 32u, 16u, 32u

	};

	static const unsigned int std140arrayStrides[] = {

		16u, 16u, 16u, 16u, 16u, 16u, 16u, 16u, 16u, 16u, 16u, 32u, 16u, 16u, 16u, 32u,
		32u, 32u, 48u, 96u, 64u, 128u, 48u, 48u, 64u, 64u, 32u, 64u, 64u, 128u, 32u, 64u, 48u, 96u

	};

	static const unsigned int std430arrayStrides[] = {

		4u, 4u, 4u, 8u, 8u, 8u, 8u, 16u, 16u, 16u, 16u, 32u, 16u, 16u, 16u, 32u,
		16u, 32u, 48u, 96u, 64u, 128u, 24u, 48u, 32u, 64u, 32u, 64u, 64u, 128u, 32u, 64u, 48u, 96u

	};

	const unsigned int* sizes = (std140) ? std140sizes : std430sizes;
	const unsigned int* alignments = (std140) ? std140alignments : std430alignments;
	const unsigned int* arrayStrides = (std140) ? std140arrayStrides : std430arrayStrides;
	const unsigned int* arrayAlignments = (std140) ? _util::std140arrayAlignments : _util::std430arrayAlignments;
	
	bool isBuffer = std140 || std430;
	bool isArray = (count > 1);
	int typeIdx = (int)dtype;

	unsigned int align;
	if (isBuffer) align = (isArray) ? arrayAlignments[typeIdx] : alignments[typeIdx];
	else align = 1u;

	int negativeOffset = -((int)offset);
	while (negativeOffset < 0) negativeOffset += (int)align;
	offset += (unsigned int)negativeOffset;
	
	if (isArray) strides[idx] = (isBuffer) ? arrayStrides[typeIdx] : uniformSizes[typeIdx];
	else strides[idx] = (isBuffer) ? sizes[typeIdx] : uniformSizes[typeIdx];

	this->uniforms[idx] = (char*)name;
	types[idx] = dtype;
	offsets[idx] = offset;
	numElements[idx] = count;
	if (IDs) IDs[idx] = glGetUniformLocation(ID, name);

}