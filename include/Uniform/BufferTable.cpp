
#include "./BufferTable.hpp"

void GL::BufferTable::bind() const {

	glBindBuffer(bufferType, ID);
	glBindBufferBase(bufferType, bindingPoint, ID);

}

void GL::BufferTable::bindToIndex(unsigned int idx) const {

	int maxBindings = (bufferType == GL_UNIFORM_BUFFER) ? _util::maxUniformBufferBindings : _util::maxShaderBufferBindings;
	if (idx >= maxBindings) throw Exception("Attempt to bind BufferTable to index " + std::to_string(idx) + ", but the maximum allowed binding point is " + std::to_string(maxBindings) + ".");

	glBindBuffer(bufferType, ID);
	glBindBufferBase(bufferType, idx, ID);

}

void GL::BufferTable::update() {

	if (!isInitialized()) throw Exception("Attempt to call the update function in an uninitialized buffer table.");
	if (first) return;

	bind();
	if (bufferNeedsAlloc) { glBufferData(bufferType, dataSize, nullptr, GL_DYNAMIC_DRAW); bufferNeedsAlloc = false; }
	glBufferSubData(bufferType, s, 1 + e - s, data + s);
	first = true;

}

GL::BufferTable::~BufferTable() { if (ID) glDeleteBuffers(1, &ID); }

GL::BufferTable::BufferTable(unsigned int bindingPoint, GLenum bufferType) : bindingPoint(bindingPoint), bufferType(bufferType) {

	int maxBindings = (bufferType == GL_UNIFORM_BUFFER) ? _util::maxUniformBufferBindings : _util::maxShaderBufferBindings;
	if (bindingPoint >= maxBindings) throw Exception("Buffer binding point supplied was " + std::to_string(bindingPoint) + ", but the maximum allowed binding point is " + std::to_string(maxBindings) + ".");

	glGenBuffers(1, &ID);

	std140 = (bufferType == GL_UNIFORM_BUFFER);
	std430 = !std140;

}

unsigned int GL::BufferTable::getset_getIndex(const char* name) const {

	unsigned int idx = numUniforms;
	for (unsigned int i = 0u; i < numUniforms; i++)

		if (!strcmp(name, uniforms[i])) {

			idx = i;
			break;

		}
	
	return idx;

}

#define _GL_BufferTable_get_commonCode_specialization(T) \
template <> \
T GL::BufferTable::get_commonCode<T>(unsigned int idx, unsigned int index) const { \
	\
	unsigned char* dataStart = data + offsets[idx] + strides[idx] * index; \
	return *(T*)(dataStart); \
	\
}

_GL_BufferTable_get_commonCode_specialization(int)
_GL_BufferTable_get_commonCode_specialization(unsigned int)
_GL_BufferTable_get_commonCode_specialization(float)

#define _GL_BufferTable_get_commonCode_vectorSpecialization(dim) \
_GL_BufferTable_get_commonCode_specialization(GL::Vector ## dim ## D<int>) \
_GL_BufferTable_get_commonCode_specialization(GL::Vector ## dim ## D<unsigned int>) \
_GL_BufferTable_get_commonCode_specialization(GL::Vector ## dim ## D<float>)

_GL_BufferTable_get_commonCode_vectorSpecialization(2)
_GL_BufferTable_get_commonCode_vectorSpecialization(3)
_GL_BufferTable_get_commonCode_vectorSpecialization(4)

unsigned int GL::BufferTable::getMatrixStride(UniformType type) const {

	if ((int)type < 16) return 0u;
	return ((std140) ? _util::std140arrayAlignments : _util::std430arrayAlignments)[(int)type];

}