#ifndef STATICELEMENTBUFFER_HPP
#define STATICELEMENTBUFFER_HPP

#include "Buffer/StaticBuffer.hpp"

namespace GL {

	class StaticElementBuffer : public StaticBuffer<unsigned int> {
	public:
		StaticElementBuffer(unsigned int length) : StaticBuffer<unsigned int>(length, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW) { }
	};

}

#endif