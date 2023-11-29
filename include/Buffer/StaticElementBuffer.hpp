#ifndef STATICELEMENTBUFFER_HPP
#define STATICELEMENTBUFFER_HPP

#include "./StaticBuffer.hpp"

namespace GL {

	class StaticElementBuffer : public StaticBuffer<unsigned int> {
	public:
		StaticElementBuffer(unsigned int length);
	};

}

#endif