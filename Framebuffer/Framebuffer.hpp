#ifndef FRAMEBUFFER_HPP
#define FRAMEBUFFER_HPP

#include "Framebuffer/RenderTexture.hpp"
#include "Framebuffer/Renderbuffer.hpp"
#include "Texture/Texture2D.hpp"

namespace GL {

	class Framebuffer : public _util {
	public:

		Framebuffer(unsigned int width, unsigned int height);
		
		bool isComplete() const;

		void bind() const;

		GLuint getID() const;

		unsigned int getWidth() const;
		unsigned int getHeight() const;

		void setColorTarget(ColorRenderTexture& renderTarget, unsigned int attachmentPoint = 0u) const;
		void setColorTarget(Texture2D& renderTarget, unsigned int attachmentPoint = 0u) const;
		void setColorTarget(ColorRenderbuffer& renderTarget, unsigned int attachmentPoint = 0u) const;

		void setDepthStencilTarget(DepthStencilRenderTexture& renderTexture) const;
		void setDepthStencilTarget(DepthStencilRenderbuffer& renderbuffer) const;

		template <typename... T>
		void use(unsigned int activeColorTarget = 0u, T... activeColorTargets);

		void useWithoutColorBuffer() const;

		static void useDefault(unsigned int screenWidth = 0u, unsigned int screenHeight = 0u);

		~Framebuffer();

	private:

		GLuint fbo = 0u;
		unsigned int w = 0u, h = 0u;

		bool* hasColorAttachment = nullptr;
		GLenum* bufs = nullptr;

		template <typename... T>
		void use_recursive(int idx, unsigned int activeColorTarget, T... activeColorTargets);
		void use_recursive(int idx, unsigned int activeColorTarget);
		void buildBufsList(int& idx, unsigned int activeColorTarget);

	};

}

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

#define _GL_Framebuffer_setColorTarget(ClassName, framebufferAttach) \
void GL::Framebuffer::setColorTarget(ClassName& renderTarget, unsigned int attachmentPoint) const { \
	\
	if ((GLint)attachmentPoint >= _util::maxColorAttachments) throw Exception("Framebuffer color target attachment point " + std::to_string(attachmentPoint) + " is too high."); \
	if (renderTarget.width() != w || renderTarget.height() != h) throw Exception("Color render target dimensions (" + std::to_string(renderTarget.width()) + ", " + std::to_string(renderTarget.height()) + ") do not match framebuffer dimensions (" + std::to_string(w) + ", " + std::to_string(h) + ")."); \
	\
	hasColorAttachment[attachmentPoint] = true; \
	\
	bind(); \
	framebufferAttach; \
	\
}

_GL_Framebuffer_setColorTarget(ColorRenderTexture, glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentPoint, renderTarget.isMultisampled() ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, renderTarget.getID(), 0))

_GL_Framebuffer_setColorTarget(Texture2D, glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentPoint, GL_TEXTURE_2D, renderTarget.getID(), 0))

_GL_Framebuffer_setColorTarget(ColorRenderbuffer, glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentPoint, GL_RENDERBUFFER, renderTarget.getID()))

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

template <typename... T>
void GL::Framebuffer::use(unsigned int activeColorTarget, T... activeColorTargets) {

	if (!isComplete()) throw Exception("Attempt to use an incomplete framebuffer.");

	bind();
	use_recursive(0, activeColorTarget, activeColorTargets...);

}

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

template <typename... T>
void GL::Framebuffer::use_recursive(int idx, unsigned int activeColorTarget, T... activeColorTargets) {

	if (idx >= _util::maxColorAttachments) throw Exception("Too many color attachment indices supplied to framebuffer.");

	buildBufsList(idx, activeColorTarget);
	use_recursive(idx, activeColorTargets...);

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

#endif