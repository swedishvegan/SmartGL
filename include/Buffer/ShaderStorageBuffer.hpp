#ifndef SHADERSTORAGEBUFFER_HPP
#define SHADERSTORAGEBUFFER_HPP

#include "./StaticBuffer.hpp"

namespace GL {

#define _GL_MakeShaderStorageBufferClass(Class, BaseClass) \
	template <typename S> \
	class Class : public BaseClass<S> { \
	public: \
		\
		Class(unsigned int length, unsigned int bindingPoint); \
		\
		void bind() const; \
		\
	protected: \
		\
		unsigned int bindingPoint; \
		\
	};

	_GL_MakeShaderStorageBufferClass(ShaderStorageBuffer, Buffer)

	_GL_MakeShaderStorageBufferClass(CoupledShaderStorageBuffer, StaticBuffer)

}

#define _GL_ImplementShaderStorageBufferClass(Class, BaseClass, GL_DRAW_MODE) \
template <typename S> \
GL::Class<S>::Class(unsigned int length, unsigned int bindingPoint) : BaseClass<S>(length, GL_SHADER_STORAGE_BUFFER, GL_STATIC_DRAW), bindingPoint(bindingPoint) { \
	\
	if (bindingPoint >= _util::maxShaderBufferBindings) throw Exception("Binding point for shader storage buffer is too high."); \
	\
	Buffer<S>::genBuffer(length); \
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, Buffer<S>::ID); \
	\
} \
\
template <typename S> \
void GL::Class<S>::bind() const { \
	\
	if (Buffer<S>::ID) { \
		\
		glBindBuffer(Buffer<S>::target, Buffer<S>::ID); \
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, Buffer<S>::ID); \
		\
	} \
	\
}

_GL_ImplementShaderStorageBufferClass(ShaderStorageBuffer, Buffer, GL_STATIC_DRAW)

_GL_ImplementShaderStorageBufferClass(CoupledShaderStorageBuffer, StaticBuffer, GL_DYNAMIC_DRAW)

#endif