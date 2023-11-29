#ifndef DYNAMICVERTEXBUFFER_HPP
#define DYNAMICVERTEXBUFFER_HPP

#include "./DynamicBuffer.hpp"

namespace GL {

	template <typename S>
	class DynamicVertexBuffer : public DynamicBuffer<S> {
	public:

		DynamicVertexBuffer(unsigned int length, unsigned int GPUlength, unsigned int numStackElements, bool emptyOnInit = false, float CPU_growRate = 1.5f, float GPU_growRate = 2.0f, float stack_growRate = 1.5f) : DynamicBuffer<S>(length, GPUlength, numStackElements, CPU_growRate, GPU_growRate, stack_growRate, emptyOnInit, GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW) { }
		DynamicVertexBuffer(unsigned int length, unsigned int GPUlength, bool emptyOnInit = false, float CPU_growRate = 1.5f, float GPU_growRate = 2.0f) : DynamicBuffer<S>(length, GPUlength, 0u, CPU_growRate, GPU_growRate, 1.5f, emptyOnInit, GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW) { }
		DynamicVertexBuffer(unsigned int length, bool emptyOnInit = false, float CPU_growRate = 1.5f, float GPU_growRate = 2.0f) : DynamicBuffer<S>(length, length, 0u, CPU_growRate, GPU_growRate, 1.5f, emptyOnInit, GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW) { }

	};

}

#endif