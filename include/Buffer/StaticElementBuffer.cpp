
#include "./StaticElementBuffer.hpp"

GL::StaticElementBuffer::StaticElementBuffer(unsigned int length) : StaticBuffer<unsigned int>(length, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW) { }