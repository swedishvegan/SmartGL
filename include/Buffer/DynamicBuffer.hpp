#ifndef DYNAMICBUFFER_HPP
#define DYNAMICBUFFER_HPP

#include <cstring>

#include "./Buffer.hpp"

namespace GL {

	template <typename S>
	class DynamicBuffer : public CoupledBuffer<S> {
	public:

		void append(S element);

		void grow(unsigned int amount);

		void remove(int start, unsigned int numElements);

		void truncate(int start = -1);

		void pushToStack(S element);

		void mergeStack(int mergeIndex = -1);

		void clear();

		void clearStack();

		~DynamicBuffer();

	protected:

		unsigned int maxLen, GPU_len, stack_len, stack_maxLen;
		unsigned char* stack_data = nullptr;

		float CPU_grow, GPU_grow, stack_grow;

		DynamicBuffer(unsigned int length, unsigned int GPUlength, unsigned int numStackElements, float CPU_growRate, float GPU_growRate, float stack_growRate, bool emptyOnInit, GLenum target, GLenum usage);

		static void growData(unsigned char** theData, unsigned int& oldLength, unsigned int newLength, bool memcopy = true);
		static unsigned int calcNewSize(unsigned int curSize, float growRate);

	};

}

template <typename S>
void GL::DynamicBuffer<S>::append(S element) {

	if (!Buffer<S>::success()) return;

	if (Buffer<S>::len == maxLen) growData(&(CoupledBuffer<S>::data), maxLen, calcNewSize(maxLen, CPU_grow));

	if (Buffer<S>::len == GPU_len) {

		GPU_len = calcNewSize(GPU_len, GPU_grow);
		Buffer<S>::bind();
		glBufferData(Buffer<S>::target, sizeof(S) * GPU_len, nullptr, Buffer<S>::usage);

		CoupledBuffer<S>::s = 0u;
		CoupledBuffer<S>::first = false;

	}

	((S*)CoupledBuffer<S>::data)[Buffer<S>::len] = element;

	if (CoupledBuffer<S>::first) {

		CoupledBuffer<S>::s = Buffer<S>::len;
		CoupledBuffer<S>::first = false;

	}

	CoupledBuffer<S>::e = Buffer<S>::len;
	Buffer<S>::len++;

}

template <typename S>
void GL::DynamicBuffer<S>::grow(unsigned int amount) {

	unsigned int newLen = Buffer<S>::len + amount;
	unsigned int newMaxLen = maxLen;

	while (newLen > newMaxLen) newMaxLen = calcNewSize(newMaxLen, CPU_grow);
	if (newMaxLen > maxLen) growData(&(CoupledBuffer<S>::data), maxLen, newMaxLen);

	if (newLen >= GPU_len) {

		while (newLen >= GPU_len) GPU_len = calcNewSize(GPU_len, GPU_grow);
		Buffer<S>::bind();
		glBufferData(Buffer<S>::target, sizeof(S) * GPU_len, nullptr, Buffer<S>::usage);

		CoupledBuffer<S>::s = 0u;
		CoupledBuffer<S>::e = newLen - amount - 1u;
		CoupledBuffer<S>::first = false;

	}

	Buffer<S>::len = newLen;

}

template <typename S>
void GL::DynamicBuffer<S>::remove(int start, unsigned int numElements) {

	if (numElements == 0u) return;

	unsigned int uDst = CoupledBuffer<S>::idxToUidx(start);
	unsigned int uSrc = uDst + numElements;

	if (uSrc >= Buffer<S>::len) {

		Buffer<S>::len = uDst;

		if (!CoupledBuffer<S>::first && CoupledBuffer<S>::e >= uDst) {

			if (CoupledBuffer<S>::s >= uDst) CoupledBuffer<S>::first = true;
			else CoupledBuffer<S>::e = uDst - 1u;

		}

	}
	else {

		S* sData = (S*)CoupledBuffer<S>::data;
		for (unsigned int i = 0u; i < Buffer<S>::len - uSrc; i++) sData[uDst + i] = sData[uSrc + i];

		Buffer<S>::len -= numElements;

		if (CoupledBuffer<S>::first) {

			CoupledBuffer<S>::s = uDst;
			CoupledBuffer<S>::first = false;

		}
		else if (uDst < CoupledBuffer<S>::s) CoupledBuffer<S>::s = uDst;
		CoupledBuffer<S>::e = Buffer<S>::len - 1u;

	}

}

template <typename S>
void GL::DynamicBuffer<S>::truncate(int start) { remove(start, Buffer<S>::len); }

template <typename S>
void GL::DynamicBuffer<S>::pushToStack(S element) {

	if (!stack_data) throw Exception("Stack was not created for this object.");

	if (stack_len == stack_maxLen) growData(&stack_data, stack_maxLen, calcNewSize(stack_maxLen, stack_grow));

	((S*)stack_data)[stack_len] = element;
	stack_len++;

}

template <typename S>
void GL::DynamicBuffer<S>::mergeStack(int mergeIndex) {

	if (!stack_data) throw Exception("Stack was not created for this object.");
	if (stack_len == 0u) return;

	unsigned int uSrc = CoupledBuffer<S>::idxToUidx(mergeIndex);
	unsigned int uDst = uSrc + stack_len;

	unsigned int newLen = Buffer<S>::len + stack_len;
	unsigned int newMaxLen = maxLen;

	while (newLen > newMaxLen) newMaxLen = calcNewSize(newMaxLen, CPU_grow);
	if (newMaxLen > maxLen) growData(&(CoupledBuffer<S>::data), maxLen, newMaxLen);

	if (newLen >= GPU_len) {

		while (newLen >= GPU_len) GPU_len = calcNewSize(GPU_len, GPU_grow);
		Buffer<S>::bind();
		glBufferData(Buffer<S>::target, sizeof(S) * GPU_len, nullptr, Buffer<S>::usage);

		CoupledBuffer<S>::s = 0u;
		CoupledBuffer<S>::first = false;

	}

	S* sData = (S*)CoupledBuffer<S>::data;
	unsigned int numMove = newLen - uDst;

	for (unsigned int i = 1u; i <= numMove; i++) sData[uDst + numMove - i] = sData[uSrc + numMove - i];

	memcpy(
		CoupledBuffer<S>::data + uSrc * sizeof(S),
		stack_data,
		stack_len * sizeof(S)
	);

	if (CoupledBuffer<S>::first) {

		CoupledBuffer<S>::s = uSrc;
		CoupledBuffer<S>::first = false;

	}
	else if (uSrc < CoupledBuffer<S>::s) CoupledBuffer<S>::s = uSrc;
	CoupledBuffer<S>::e = newLen - 1u;

	CoupledBuffer<S>::len = newLen;
	stack_len = 0u;

}

template <typename S>
void GL::DynamicBuffer<S>::clear() {

	Buffer<S>::len = 0u;
	CoupledBuffer<S>::first = true;

}

template <typename S>
void GL::DynamicBuffer<S>::clearStack() { stack_len = 0u; }

template <typename S>
GL::DynamicBuffer<S>::~DynamicBuffer() { if (stack_data) delete[] stack_data; }

template <typename S>
GL::DynamicBuffer<S>::DynamicBuffer(unsigned int length, unsigned int GPUlength, unsigned int numStackElements, float CPU_growRate, float GPU_growRate, float stack_growRate, bool emptyOnInit, GLenum target, GLenum usage) : CoupledBuffer<S>(emptyOnInit ? 0u : length, length, target, usage) {
	
	if (!GPUlength) throw Exception("GPU buffer length is zero.");

	if (CPU_growRate < 1.1f) CPU_growRate = 1.1f;
	if (GPU_growRate < 1.1f) GPU_growRate = 1.1f;
	if (stack_growRate < 1.1f) stack_growRate = 1.1f;

	maxLen = length; GPU_len = GPUlength; stack_len = 0u; stack_maxLen = numStackElements;
	CPU_grow = CPU_growRate; GPU_grow = GPU_growRate; stack_grow = stack_growRate;

	if (numStackElements) stack_data = new unsigned char[numStackElements * sizeof(S)];

	Buffer<S>::genBuffer(GPUlength);

}

template <typename S>
void GL::DynamicBuffer<S>::growData(unsigned char** theData, unsigned int& oldLength, unsigned int newLength, bool memcopy) {

	unsigned char* newData = new unsigned char[newLength * sizeof(S)];
	if (memcopy) std::memcpy(
		newData,
		*theData,
		oldLength * sizeof(S)
	);
	delete[] * theData;
	*theData = newData;
	oldLength = newLength;

}

template <typename S>
unsigned int GL::DynamicBuffer<S>::calcNewSize(unsigned int curSize, float growRate) { return (unsigned int)((float)curSize * growRate) + 1u; }

#endif