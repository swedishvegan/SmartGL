#ifndef STATICBUFFER_HPP
#define STATICBUFFER_HPP

#include "./Buffer.hpp"

namespace GL {

	template <typename S>
	class StaticBuffer : public CoupledBuffer<S> {
	public:

		void resize(unsigned int newSize, bool copyOldData = true);

	protected:

		StaticBuffer(unsigned int length, GLenum target, GLenum usage);

	};

}

template <typename S>
void GL::StaticBuffer<S>::resize(unsigned int newSize, bool copyOldData) {

	if (newSize == 0u) throw Exception("Attempt to resize static buffer to length zero.");

	unsigned char* newData = new unsigned char[newSize * sizeof(S)];
	if (copyOldData) {

		unsigned int l = (Buffer<S>::len > newSize) ? newSize : Buffer<S>::len;
		memcpy(
			newData,
			CoupledBuffer<S>::data,
			sizeof(S) * l
		);

	}

	Buffer<S>::bind();
	glBufferData(Buffer<S>::target, sizeof(S) * newSize, (copyOldData) ? newData : nullptr, Buffer<S>::usage);

	delete[] CoupledBuffer<S>::data;
	CoupledBuffer<S>::data = newData;
	Buffer<S>::len = newSize;
	CoupledBuffer<S>::first = true;

}

template <typename S>
GL::StaticBuffer<S>::StaticBuffer(unsigned int length, GLenum target, GLenum usage) : CoupledBuffer<S>(length, length, target, usage) { Buffer<S>::genBuffer(length); }

#endif