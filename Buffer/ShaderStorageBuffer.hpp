#ifndef SHADERSTORAGEBUFFER_HPP
#define SHADERSTORAGEBUFFER_HPP

#include "Buffer/StaticBuffer.hpp"

namespace GL {

	template <typename S>
	class ShaderStorageBuffer : public Buffer<S> {
	public:
		ShaderStorageBuffer(unsigned int length, unsigned int bindingPoint);
	};

	template <typename S>
	class CoupledShaderStorageBuffer : public StaticBuffer<S> {
	public:
		CoupledShaderStorageBuffer(unsigned int length, unsigned int bindingPoint);
	};

}

template <typename S>
GL::ShaderStorageBuffer<S>::ShaderStorageBuffer(unsigned int length, unsigned int bindingPoint) : Buffer<S>(length, GL_SHADER_STORAGE_BUFFER, GL_STATIC_DRAW) {

	if (bindingPoint >= _util::maxBindings) throw Exception("Binding point for coupled shader storage buffer is too high.");

	Buffer<S>::genBuffer(length);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, Buffer<S>::ID);

}

template <typename S>
GL::CoupledShaderStorageBuffer<S>::CoupledShaderStorageBuffer(unsigned int length, unsigned int bindingPoint) : StaticBuffer<S>(length, GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW) {
	
	if (bindingPoint >= _util::maxBindings) throw Exception("Binding point for coupled shader storage buffer is too high.");

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, Buffer<S>::ID);

}

#endif