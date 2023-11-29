#ifndef BUFFER_HPP
#define BUFFER_HPP

#include "./../util/util.hpp"

namespace GL {

	template <typename S>
	class Buffer : public _util {
	public:

		virtual void bind() const;

		unsigned int getLength() const;
		GLuint getID();

		void writeDataToGPU(S* data, int start, unsigned int numElements) const;

		void readDataFromGPU(S* dst, int start, unsigned int numElements) const;
		void readDataFromGPU(S* dst, int start = 0) const;

		~Buffer();

	protected:

		GLuint ID = 0u;
		GLenum target;
		GLenum usage;
		unsigned int len = 0u;

		Buffer(unsigned int length, GLenum target, GLenum usage);

		void genBuffer(unsigned int bufLength);

		void def_writeToGPU(S* data, int start, unsigned int numElements) const;

		unsigned int idxToUidx(int idx) const;

	};

	template <typename S>
	class CoupledBuffer : public Buffer<S> {
	public:

		void writeToGPU();
		void writeToGPU(S* data, int start, unsigned int numElements);

		void readFromGPU(int start, unsigned int numElements);
		void readFromGPU(int start = 0);

		S operator () (int index) const;
		S& operator [] (int index);

		~CoupledBuffer();

	protected:

		unsigned char* data = nullptr;
		unsigned int s, e;
		bool first = true;

		CoupledBuffer(unsigned int length, unsigned int CPUalloc, GLenum target, GLenum usage);

	};

}

template <typename S>
void GL::Buffer<S>::bind () const { if (ID) glBindBuffer(target, ID); }

template <typename S>
unsigned int GL::Buffer<S>::getLength() const { return len; }

template <typename S>
GLuint GL::Buffer<S>::getID() { return ID; }

template <typename S>
void GL::Buffer<S>::writeDataToGPU(S* data, int start, unsigned int numElements) const { def_writeToGPU(data, start, numElements); }

template <typename S>
void GL::Buffer<S>::readDataFromGPU(S* dst, int start, unsigned int numElements) const {

	if (!dst) throw Exception("Destination for buffer read is null.");

	unsigned int uStart = idxToUidx(start);
	if (uStart + numElements > len) numElements = len - uStart;
	if (numElements == 0u) return;

	glBindBuffer(target, ID);
	glGetBufferSubData(target, uStart * sizeof(S), numElements * sizeof(S), dst);

}

template <typename S>
void GL::Buffer<S>::readDataFromGPU(S* dst, int start) const { readFromGPU(dst, start, len); }

template <typename S>
GL::Buffer<S>::~Buffer() { if (ID) glDeleteBuffers(1, &ID); }

template <typename S>
GL::Buffer<S>::Buffer(unsigned int length, GLenum target, GLenum usage) : len(length), target(target), usage(usage) { }

template <typename S>
void GL::Buffer<S>::genBuffer(unsigned int bufLength) {

	glGenBuffers(1, &ID);
	glBindBuffer(target, ID);
	glBufferData(target, bufLength * sizeof(S), nullptr, usage);

}

template <typename S>
void GL::Buffer<S>::def_writeToGPU(S* data, int start, unsigned int numElements) const {

	unsigned int uStart = idxToUidx(start);
	if (start + numElements > len) numElements = len - uStart;
	if (numElements == 0u) return;

	glBindBuffer(target, ID);
	glBufferSubData(target, sizeof(S) * uStart, sizeof(S) * numElements, data);

}

template <typename S>
unsigned int GL::Buffer<S>::idxToUidx(int idx) const { return ((idx < 0) ? Buffer<S>::len - (unsigned int)(-idx) : (unsigned int)idx) % Buffer<S>::len; }

template <typename S>
void GL::CoupledBuffer<S>::writeToGPU() {

	if (first) return;

	Buffer<S>::def_writeToGPU(data + s * sizeof(S), s, 1 + e - s);
	first = true;

}

template <typename S>
void GL::CoupledBuffer<S>::writeToGPU(S* data, int start, unsigned int numElements) {

	unsigned int uStart = Buffer<S>::idxToUidx(start);
	if (start + numElements > Buffer<S>::len) numElements = Buffer<S>::len - uStart;
	if (numElements == 0u) return;

	glBindBuffer(Buffer<S>::target, Buffer<S>::ID);
	glBufferSubData(Buffer<S>::target, sizeof(S) * uStart, sizeof(S) * numElements, data);

	memcpy(
		this->data + sizeof(S) * uStart,
		data,
		sizeof(S) * numElements
	);

	if (!first) {

		unsigned int end = uStart + numElements - 1u;

		bool sInRange = (s >= uStart && s <= end);
		bool eInRange = (e >= uStart && e <= end);

		if (s < uStart && eInRange) e = uStart;
		else if (e > end && sInRange) s = end;
		else if (sInRange && eInRange) first = true;

	}

}

template <typename S>
void GL::CoupledBuffer<S>::readFromGPU(int start, unsigned int numElements) {

	unsigned int uStart = Buffer<S>::idxToUidx(start);
	if (uStart + numElements > Buffer<S>::len) numElements = Buffer<S>::len - uStart;
	if (numElements == 0u) return;

	glBindBuffer(Buffer<S>::target, Buffer<S>::ID);
	glGetBufferSubData(Buffer<S>::target, uStart * sizeof(S), numElements * sizeof(S), data + uStart * sizeof(S));

	if (!first) {

		unsigned int end = uStart + numElements - 1u;

		bool sInRange = (s >= uStart && s <= end);
		bool eInRange = (e >= uStart && e <= end);

		if (s < uStart && eInRange) e = uStart;
		else if (e > end && sInRange) s = end;
		else if (sInRange && eInRange) first = true;

	}

}

template <typename S>
void GL::CoupledBuffer<S>::readFromGPU(int start) { readFromGPU(start, Buffer<S>::len); }

template <typename S>
S GL::CoupledBuffer<S>::operator () (int index) const {

	unsigned int uIdx = Buffer<S>::idxToUidx(index);
	return ((S*)data)[uIdx];

}

template <typename S>
S& GL::CoupledBuffer<S>::operator [] (int index) {

	unsigned int uIdx = Buffer<S>::idxToUidx(index);

	if (first) {

		s = uIdx; e = uIdx;
		first = false;

	}
	else {

		if (uIdx < s) s = uIdx;
		if (uIdx > e) e = uIdx;

	}

	return ((S*)data)[uIdx];

}

template <typename S>
GL::CoupledBuffer<S>::~CoupledBuffer() { if (data) delete[] data; }

template <typename S>
GL::CoupledBuffer<S>::CoupledBuffer(unsigned int length, unsigned int CPUalloc, GLenum target, GLenum usage) : Buffer<S>(length, target, usage) {
	
	if (CPUalloc == 0u) throw Exception("CPU buffer length is zero.");

	data = new unsigned char[CPUalloc * sizeof(S)];

}

#endif