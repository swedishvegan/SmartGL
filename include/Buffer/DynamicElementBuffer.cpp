
#include "./DynamicElementBuffer.hpp"

GL::DynamicElementBuffer::DynamicElementBuffer(unsigned int length, unsigned int GPUlength, unsigned int numStackElements, bool emptyOnInit, float CPU_growRate, float GPU_growRate, float stack_growRate) : DynamicBuffer<unsigned int>(length, GPUlength, numStackElements, CPU_growRate, GPU_growRate, stack_growRate, emptyOnInit, GL_ELEMENT_ARRAY_BUFFER, GL_DYNAMIC_DRAW) { }
GL::DynamicElementBuffer::DynamicElementBuffer(unsigned int length, unsigned int GPUlength, bool emptyOnInit, float CPU_growRate, float GPU_growRate) : DynamicBuffer<unsigned int>(length, GPUlength, 0u, CPU_growRate, GPU_growRate, 1.5f, emptyOnInit, GL_ELEMENT_ARRAY_BUFFER, GL_DYNAMIC_DRAW) { }
GL::DynamicElementBuffer::DynamicElementBuffer(unsigned int length, bool emptyOnInit, float CPU_growRate, float GPU_growRate) : DynamicBuffer<unsigned int>(length, length, 0u, CPU_growRate, GPU_growRate, 1.5f, emptyOnInit, GL_ELEMENT_ARRAY_BUFFER, GL_DYNAMIC_DRAW) { }