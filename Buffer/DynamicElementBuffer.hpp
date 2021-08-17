#ifndef DYNAMICELEMENTBUFFER_HPP
#define DYNAMICELEMENTBUFFER_HPP

#include "Buffer/DynamicBuffer.hpp"

namespace GL {

	class DynamicElementBuffer : public DynamicBuffer<unsigned int> {
	public:

		DynamicElementBuffer(unsigned int length, unsigned int GPUlength, unsigned int numStackElements, bool emptyOnInit = false, float CPU_growRate = 1.5f, float GPU_growRate = 2.0f, float stack_growRate = 1.5f) : DynamicBuffer<unsigned int>(length, GPUlength, numStackElements, CPU_growRate, GPU_growRate, stack_growRate, emptyOnInit, GL_ELEMENT_ARRAY_BUFFER, GL_DYNAMIC_DRAW) { }
		DynamicElementBuffer(unsigned int length, unsigned int GPUlength, bool emptyOnInit = false, float CPU_growRate = 1.5f, float GPU_growRate = 2.0f) : DynamicBuffer<unsigned int>(length, GPUlength, 0u, CPU_growRate, GPU_growRate, 1.5f, emptyOnInit, GL_ELEMENT_ARRAY_BUFFER, GL_DYNAMIC_DRAW) { }
		DynamicElementBuffer(unsigned int length, bool emptyOnInit = false, float CPU_growRate = 1.5f, float GPU_growRate = 2.0f) : DynamicBuffer<unsigned int>(length, length, 0u, CPU_growRate, GPU_growRate, 1.5f, emptyOnInit, GL_ELEMENT_ARRAY_BUFFER, GL_DYNAMIC_DRAW) { }

	};

}

#endif