
#include "./Framebuffer.hpp"

GL::Framebuffer::Framebuffer(unsigned int width, unsigned int height) {

	if (width == 0u) throw Exception("Framebuffer width is zero.");
	if (height == 0u) throw Exception("Framebuffer height is zero.");

	w = width; h = height;

	glGenFramebuffers(1, &fbo);
	bind();

	hasColorAttachment = new bool[_util::maxColorAttachments];
	bufs = new GLenum[_util::maxColorAttachments];

	for (int i = 0; i < _util::maxColorAttachments; i++) hasColorAttachment[i] = false;

}

bool GL::Framebuffer::isComplete() const {

	bind();
	return (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

}

void GL::Framebuffer::bind() const { 
	
	glBindFramebuffer(GL_FRAMEBUFFER, fbo); 
	glViewport(0, 0, (int)w, (int)h);

}

GLuint GL::Framebuffer::getID() const { return fbo; }

unsigned int GL::Framebuffer::getWidth() const { return w; }

unsigned int GL::Framebuffer::getHeight() const { return h; }

_GL_Framebuffer_setColorTarget(ColorRenderTexture, width, height, _va_args(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentPoint, renderTarget.isMultisampled() ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, renderTarget.getID(), 0)),)

_GL_Framebuffer_setColorTarget(Texture2D, width, height, _va_args(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentPoint, GL_TEXTURE_2D, renderTarget.getID(), 0)),)

_GL_Framebuffer_setColorTarget(ColorRenderbuffer, width, height, _va_args(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentPoint, GL_RENDERBUFFER, renderTarget.getID())),)

#define _GL_Framebuffer_setDepthStencilTarget(ClassName, framebufferAttach) \
void GL::Framebuffer::setDepthStencilTarget(ClassName& renderTarget) const { \
	\
	if (renderTarget.width() != w || renderTarget.height() != h) throw Exception("Depth-stencil target dimensions (" + std::to_string(renderTarget.width()) + ", " + std::to_string(renderTarget.height()) + ") do not match framebuffer dimensions (" + std::to_string(w) + ", " + std::to_string(h) + ")."); \
	\
	bind(); \
	framebufferAttach; \
	\
}

_GL_Framebuffer_setDepthStencilTarget(DepthStencilRenderTexture, glFramebufferTexture2D(GL_FRAMEBUFFER, renderTarget.isStencilTarget() ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT, renderTarget.isMultisampled() ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, renderTarget.getID(), 0))

_GL_Framebuffer_setDepthStencilTarget(DepthStencilRenderbuffer, glFramebufferRenderbuffer(GL_FRAMEBUFFER, renderTarget.isStencilTarget() ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderTarget.getID()))

void GL::Framebuffer::useWithoutColorBuffer() const {

	if (!isComplete()) throw Exception("Attempt to use an incomplete framebuffer.");

	bind();
	glDrawBuffer(GL_NONE);

}

void GL::Framebuffer::useDefault(unsigned int screenWidth, unsigned int screenHeight) {

	if (screenWidth * screenHeight == 0u) {

		screenWidth = _util::screenWidth;
		screenHeight = _util::screenHeight;

	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, screenWidth, screenHeight);

}

GL::Framebuffer::~Framebuffer() {

	if (fbo) glDeleteFramebuffers(1, &fbo);
	if (hasColorAttachment) delete[] hasColorAttachment;
	if (bufs) delete[] bufs;

}

void GL::Framebuffer::use_recursive(int idx, unsigned int activeColorTarget) {

	if (idx >= _util::maxColorAttachments) throw Exception("Too many color attachment indices were supplied to framebuffer.");

	buildBufsList(idx, activeColorTarget);
	glDrawBuffers(idx, bufs);

}

void GL::Framebuffer::buildBufsList(int& idx, unsigned int activeColorTarget) {
	
	if (activeColorTarget >= _util::maxColorAttachments) throw Exception("Framebuffer color target " + std::to_string(activeColorTarget) + " is too high.");
	if (!hasColorAttachment[activeColorTarget]) throw Exception("Attempt to use unspecified framebuffer color target " + std::to_string(activeColorTarget) + ".");
	else {

		GLenum glEnum = GL_COLOR_ATTACHMENT0 + activeColorTarget;

		for (int i = 0; i < idx; i++)
			if (bufs[i] == glEnum) throw Exception("Attempt to use color target " + std::to_string(activeColorTarget) + " twice.");

		bufs[idx] = glEnum;
		idx++;

	}

}