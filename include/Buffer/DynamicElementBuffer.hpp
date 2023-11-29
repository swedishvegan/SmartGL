#ifndef DYNAMICELEMENTBUFFER_HPP
#define DYNAMICELEMENTBUFFER_HPP

#include "./DynamicBuffer.hpp"

namespace GL {

	class DynamicElementBuffer : public DynamicBuffer<unsigned int> {
	public:

		DynamicElementBuffer(unsigned int length, unsigned int GPUlength, unsigned int numStackElements, bool emptyOnInit = false, float CPU_growRate = 1.5f, float GPU_growRate = 2.0f, float stack_growRate = 1.5f);
		DynamicElementBuffer(unsigned int length, unsigned int GPUlength, bool emptyOnInit = false, float CPU_growRate = 1.5f, float GPU_growRate = 2.0f);
		DynamicElementBuffer(unsigned int length, bool emptyOnInit = false, float CPU_growRate = 1.5f, float GPU_growRate = 2.0f);

	};

}

#endif