#ifndef RENDERBUFFER_HPP
#define RENDERBUFFER_HPP

#include "Framebuffer/RenderTarget.hpp"

namespace GL {

	class Renderbuffer : public RenderTarget {
	public:

		void bind() const;

		GLuint getID() const;

	protected:

		GLuint ID;

		Renderbuffer(unsigned int width, unsigned int height, bool multisampled, ColorFormat colorFormat, DataType colorDataType, DepthStencilFormat depthStencilFormat, bool isColorBuffer);

		~Renderbuffer();

	};

	class ColorRenderbuffer : public Renderbuffer {
	public:
		ColorRenderbuffer(unsigned int width, unsigned int height, bool multisampled = false, ColorFormat format = ColorFormat::RGBA, DataType type = DataType::F32)
			: Renderbuffer(width, height, multisampled, format, type, DepthStencilFormat::DEPTH_16, true) { }
	};

	class DepthStencilRenderbuffer : public Renderbuffer {
	public:
		DepthStencilRenderbuffer(unsigned int width, unsigned int height, bool multisampled = false, DepthStencilFormat format = DepthStencilFormat::DEPTH_24)
			: Renderbuffer(width, height, multisampled, ColorFormat::R, DataType::U8, format, false) { }
	};

}

void GL::Renderbuffer::bind() const { glBindRenderbuffer(GL_RENDERBUFFER, ID); }

GLuint GL::Renderbuffer::getID() const { return ID; }

GL::Renderbuffer::Renderbuffer(unsigned int width, unsigned int height, bool multisampled, ColorFormat colorFormat, DataType colorDataType, DepthStencilFormat depthStencilFormat, bool isColorBuffer) {

	if (width == 0u) throw Exception("Renderbuffer width is zero.");
	if (height == 0u) throw Exception("Renderbuffer height is zero.");

	w = width; h = height;
	isTex = false;
	isMulti = multisampled;

	isColor = isColorBuffer;
	hasStencil = (depthStencilFormat == DepthStencilFormat::DEPTH_24_STENCIL_8 || depthStencilFormat == DepthStencilFormat::DEPTH_32_STENCIL_8);

	glGenRenderbuffers(1, &ID);
	bind();
	int samples = (multisampled) ? 4 : 0;
	if (isColor) glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, _util::colorStorageTypes[4 * (int)colorDataType + (int)colorFormat], w, h);
	else glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, _util::depthStencilStorageTypes[(int)depthStencilFormat], w, h);

}

GL::Renderbuffer::~Renderbuffer() { if (ID) glDeleteRenderbuffers(1, &ID); }

#endif