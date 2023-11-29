
#include "./Renderbuffer.hpp"

GL::ColorRenderbuffer::ColorRenderbuffer(unsigned int width, unsigned int height, bool multisampled, ColorFormat format, DataType type)
	: Renderbuffer(width, height, multisampled, format, type, DepthStencilFormat::DEPTH_16, true), format(format), dataType(type) { }

GL::ColorFormat GL::ColorRenderbuffer::getColorFormat() const { return format; }

GL::DataType GL::ColorRenderbuffer::getInternalDataType() const { return dataType; }

GL::DepthStencilRenderbuffer::DepthStencilRenderbuffer(unsigned int width, unsigned int height, bool multisampled, DepthStencilFormat format)
    : Renderbuffer(width, height, multisampled, ColorFormat::R, DataType::U8, format, false), format(format) { }

GL::DepthStencilFormat GL::DepthStencilRenderbuffer::getDepthStencilFormat() const { return format; }

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