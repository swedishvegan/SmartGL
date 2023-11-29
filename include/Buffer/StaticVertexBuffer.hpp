#ifndef STATICVERTEXBUFFER_HPP
#define STATICVERTEXBUFFER_HPP

#include "./StaticBuffer.hpp"

namespace GL {

	template <typename S>
	class StaticVertexBuffer : public StaticBuffer<S> {
	public:
		StaticVertexBuffer(unsigned int length) : StaticBuffer<S>(length, GL_ARRAY_BUFFER, GL_STATIC_DRAW) { }
	};

}

#endif