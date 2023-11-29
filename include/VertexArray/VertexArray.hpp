#ifndef VERTEXARRAY_HPP
#define VERTEXARRAY_HPP

#include <type_traits>

#include "./../Buffer/StaticVertexBuffer.hpp"
#include "./../Buffer/DynamicVertexBuffer.hpp"
#include "./../Buffer/StaticElementBuffer.hpp"
#include "./../Buffer/DynamicElementBuffer.hpp"
#include "./../util/enums.hpp"

namespace GL {

	struct VertexAttribute {
	public:

		VertexAttribute(DataType dataType, unsigned int numComponents, bool convertToFloat = false, bool normalize = true);

		DataType getDataType() const;
		unsigned int getNumComponents() const;
		unsigned int getSize() const;
		bool convertToFloat() const;
		bool normalize() const;

	private:

		DataType dataType;
		unsigned int numComponents;
		unsigned int size;
		bool convert;
		bool norm;

	};

	template <typename S = char>
	class VertexArray : public _util {
	public:

		VertexArray();

		void setElementBuffer(StaticElementBuffer& buffer);
		void setElementBuffer(DynamicElementBuffer& buffer);
		
		void draw(int start, unsigned int numVertices, DrawMode mode = DrawMode::TRIANGLES, bool instanced = false, unsigned int numInstances = 1u) const;
		void draw(int start = 0, DrawMode mode = DrawMode::TRIANGLES, bool instanced = false, unsigned int numInstances = 1u) const;
		void drawElements(unsigned int numVertices, DrawMode mode = DrawMode::TRIANGLES, bool instanced = false, unsigned int numInstances = 1u) const;
		void drawElements(DrawMode = DrawMode::TRIANGLES, bool instanced = false, unsigned int numInstances = 1u) const;

		static void setClearColor(vec4 color);

		static void clearBuffers(int GL_BufferType_bufferFlags = COLOR_BUFFER | DEPTH_BUFFER);

		template <typename T>
		static void clearColorBuffer(Vector4D<T> color = Vector4D<T>(), unsigned int attachmentPoint = 0u);

		static void drawDefault(unsigned int start = 0u, unsigned int numVertices = 6u, DrawMode mode = DrawMode::TRIANGLES, bool instanced = false, unsigned int numInstances = 1u);

		template <typename... T>
		void setAttributes(StaticVertexBuffer<S>& vertexBuffer, VertexAttribute attrib, T... attribs);

		template <typename... T>
		void setAttributes(DynamicVertexBuffer<S>& vertexBuffer, VertexAttribute attrib, T... attribs);

		static void setCullMode(CullMode mode);

		~VertexArray();

	private:

		GLuint vao = 0u;
		unsigned int numEls = 0u;
		unsigned int len = 0u;
		bool hasAttributes = false;

		template <typename... T>
		void setAttributes_recursive(int idx, int totalSize, VertexAttribute& attrib, T... attribs);
		void setAttributes_recursive(int idx, int totalSize, VertexAttribute& attrib);
		void addAttribute(int idx, int& totalSize, VertexAttribute& attrib);

	};

	typedef VertexArray<> Render;

	const VertexAttribute VA_float(DataType::F32, 1);
	const VertexAttribute VA_int(DataType::I32, 1);
	const VertexAttribute VA_uint(DataType::U32, 1);

	const VertexAttribute VA_vec2(DataType::F32, 2);
	const VertexAttribute VA_ivec2(DataType::I32, 2);
	const VertexAttribute VA_uvec2(DataType::U32, 2);

	const VertexAttribute VA_vec3(DataType::F32, 3);
	const VertexAttribute VA_ivec3(DataType::I32, 3);
	const VertexAttribute VA_uvec3(DataType::U32, 3);

	const VertexAttribute VA_vec4(DataType::F32, 4);
	const VertexAttribute VA_ivec4(DataType::I32, 4);
	const VertexAttribute VA_uvec4(DataType::U32, 4);

}

template <typename S>
GL::VertexArray<S>::VertexArray() {

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

}

template <typename S>
void GL::VertexArray<S>::setElementBuffer(StaticElementBuffer& buffer) {

	glBindVertexArray(vao);
	buffer.bind();
	numEls = buffer.getLength();

}

template <typename S>
void GL::VertexArray<S>::setElementBuffer(DynamicElementBuffer& buffer) {

	glBindVertexArray(vao);
	buffer.bind();
	numEls = buffer.getLength();

}

template <typename S>
void GL::VertexArray<S>::draw(int start, unsigned int numVertices, DrawMode mode, bool instanced, unsigned int numInstances) const {
	
	if (numVertices == 0u) return;
	if (!hasAttributes) throw Exception("Attempt to draw vertex array before setting vertex attributes.");
	
	unsigned int uStart = ((start < 0) ? len - (unsigned int)(-start) : (unsigned int)start) % len;
	if (uStart + numVertices > len) numVertices = len - uStart;

	glBindVertexArray(vao);

	if (instanced) glDrawArraysInstanced(_util::drawModes[(int)mode], uStart, numVertices, numInstances);
	else glDrawArrays(_util::drawModes[(int)mode], uStart, numVertices);

}

template <typename S>
void GL::VertexArray<S>::draw(int start, DrawMode mode, bool instanced, unsigned int numInstances) const { draw(start, len, mode, instanced, numInstances); }

template <typename S>
void GL::VertexArray<S>::setClearColor(GL::vec4 color) { glClearColor(color.x, color.y, color.z, color.w); }

template <typename S>
void GL::VertexArray<S>::clearBuffers(int GL_BufferType_bufferFlags) {

	GLbitfield mask = 0u;

	if (GL_BufferType_bufferFlags % 2) mask |= GL_COLOR_BUFFER_BIT;
	if ((GL_BufferType_bufferFlags >> 1) % 2) mask |= GL_DEPTH_BUFFER_BIT;
	if ((GL_BufferType_bufferFlags >> 2) % 2) mask |= GL_STENCIL_BUFFER_BIT;

	if (mask) glClear(mask);

}

template <typename S>
template <typename T>
void GL::VertexArray<S>::clearColorBuffer(GL::Vector4D<T> color, unsigned int attachmentPoint) {

	if (std::is_same<T, GLfloat>::value) glClearBufferfv(GL_COLOR, (GLint)attachmentPoint, (const GLfloat*)(&color));
	else if (std::is_same<T, GLint>::value) glClearBufferiv(GL_COLOR, (GLint)attachmentPoint, (const GLint*)(&color));
	else if (std::is_same<T, GLuint>::value) glClearBufferuiv(GL_COLOR, (GLint)attachmentPoint, (const GLuint*)(&color));
	else throw Exception("A color buffer can only be cleared with a vec4, ivec4, or uvec4 type.");

}

template <typename S>
void GL::VertexArray<S>::drawDefault(unsigned int start, unsigned int numVertices, DrawMode mode, bool instanced, unsigned int numInstances) {

	if (numVertices == 0u) return;

	if (_util::dummyVao) glBindVertexArray(_util::dummyVao);
	if (instanced) glDrawArraysInstanced(_util::drawModes[(int)mode], start, numVertices, numInstances);
	else glDrawArrays(_util::drawModes[(int)mode], start, numVertices);

}

template <typename S>
void GL::VertexArray<S>::drawElements(unsigned int numVertices, DrawMode mode, bool instanced, unsigned int numInstances) const {
	
	if (numVertices == 0u) return;
	if (numEls == 0u) throw Exception("Attempt to call drawElements on vertex array without supplying an element buffer.");
	if (!hasAttributes) throw Exception("Attempt to draw vertex array before setting vertex attributes.");

	if (numVertices > numEls) numVertices = numEls;

	glBindVertexArray(vao);

	if (instanced) glDrawElementsInstanced(_util::drawModes[(int)mode], numVertices, GL_UNSIGNED_INT, nullptr, numInstances);
	else glDrawElements(_util::drawModes[(int)mode], numVertices, GL_UNSIGNED_INT, nullptr);

}

template <typename S>
void GL::VertexArray<S>::drawElements(DrawMode mode, bool instanced, unsigned int numInstances) const { drawElements(numEls, mode, instanced, numInstances); }

template <typename S>
template <typename... T>
void GL::VertexArray<S>::setAttributes(StaticVertexBuffer<S>& vertexBuffer, VertexAttribute attrib, T... attribs) {

	GLuint vbo = vertexBuffer.getID();
	len = vertexBuffer.length();

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	setAttributes_recursive(0, 0, attrib, attribs...);
	hasAttributes = true;

}

template <typename S>
template <typename... T>
void GL::VertexArray<S>::setAttributes(DynamicVertexBuffer<S>& vertexBuffer, VertexAttribute attrib, T... attribs) {

	GLuint vbo = vertexBuffer.getID();
	len = vertexBuffer.getLength();

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	setAttributes_recursive(0, 0, attrib, attribs...);
	hasAttributes = true;

}

template <typename S>
void GL::VertexArray<S>::setCullMode(CullMode mode) {

	if (mode == CullMode::NONE) glDisable(GL_CULL_FACE);
	else if (mode == CullMode::BACK_FACE) { glEnable(GL_CULL_FACE); glCullFace(GL_BACK); }
	else { glEnable(GL_CULL_FACE); glCullFace(GL_FRONT); }

}

template <typename S>
GL::VertexArray<S>::~VertexArray() { if (vao) glDeleteVertexArrays(1, &vao); }

template <typename S>
template <typename... T>
void GL::VertexArray<S>::setAttributes_recursive(int idx, int totalSize, VertexAttribute& attrib, T... attribs) {
	
	addAttribute(idx, totalSize, attrib);
	setAttributes_recursive(idx + 1, totalSize, attribs...);

}

template <typename S>
void GL::VertexArray<S>::setAttributes_recursive(int idx, int totalSize, VertexAttribute& attrib) {
	
	addAttribute(idx, totalSize, attrib);
	if (totalSize != sizeof(S)) throw Exception("Incorrect vertex attribute total size in vertex array.");

}

template <typename S>
void GL::VertexArray<S>::addAttribute(int idx, int& totalSize, VertexAttribute& attrib) {

	if (idx == _util::maxVertexAttribs) throw Exception("Too many vertex attributes passed to vertex array.");

	unsigned int size = attrib.getSize();

	static GLenum dataTypes[] = { GL_HALF_FLOAT, GL_FLOAT, GL_BYTE, GL_SHORT, GL_INT, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT };
	GLenum dataTypeGL = dataTypes[(int)attrib.getDataType()];

	if (attrib.getDataType() == DataType::F16) throw Exception("Attempt to pass GL::DataType::F16 as a vertex attribute data type.");

	if (attrib.convertToFloat() || dataTypeGL == GL_FLOAT) glVertexAttribPointer((GLuint)idx, (GLint)attrib.getNumComponents(), dataTypeGL, (attrib.normalize()) ? GL_TRUE : GL_FALSE, sizeof(S), (void*)(totalSize));
	else glVertexAttribIPointer((GLuint)idx, (GLint)attrib.getNumComponents(), dataTypeGL, sizeof(S), (void*)(totalSize));
	glEnableVertexAttribArray((GLuint)idx);

	totalSize += size;

}

#endif