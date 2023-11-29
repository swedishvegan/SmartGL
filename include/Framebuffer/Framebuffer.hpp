#ifndef FRAMEBUFFER_HPP
#define FRAMEBUFFER_HPP

#include "./RenderTexture.hpp"
#include "./Renderbuffer.hpp"
#include "./../Texture/Texture2D.hpp"
#include "./../Texture/TextureCubeMap.hpp"

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
		template <typename S>
		void setColorTarget(TextureCubeMap<S>& renderTarget, CubeMapFace face, unsigned int attachmentPoint = 0u) const;
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

#define _GL_Framebuffer_setColorTarget(ClassName, wid, hgt, framebufferAttach, additionalArg) \
void GL::Framebuffer::setColorTarget(ClassName& renderTarget, additionalArg unsigned int attachmentPoint) const { \
	\
	if ((GLint)attachmentPoint >= _util::maxColorAttachments) throw Exception("Framebuffer color target attachment point " + std::to_string(attachmentPoint) + " is too high."); \
	if (renderTarget.wid() != w || renderTarget.hgt() != h) throw Exception("Color render target dimensions (" + std::to_string(renderTarget.wid()) + ", " + std::to_string(renderTarget.hgt()) + ") do not match framebuffer dimensions (" + std::to_string(w) + ", " + std::to_string(h) + ")."); \
	\
	hasColorAttachment[attachmentPoint] = true; \
	\
	bind(); \
	framebufferAttach; \
	\
}

template <typename S>
_GL_Framebuffer_setColorTarget(TextureCubeMap<S>, sideLength, sideLength, _va_args(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentPoint, GL_TEXTURE_CUBE_MAP_POSITIVE_X + (int)face, renderTarget.getID(), 0);), _va_args(CubeMapFace face,))

template <typename... T>
void GL::Framebuffer::use(unsigned int activeColorTarget, T... activeColorTargets) {

	if (!isComplete()) throw Exception("Attempt to use an incomplete framebuffer.");

	bind();
	use_recursive(0, activeColorTarget, activeColorTargets...);

}

template <typename... T>
void GL::Framebuffer::use_recursive(int idx, unsigned int activeColorTarget, T... activeColorTargets) {

	if (idx >= _util::maxColorAttachments) throw Exception("Too many color attachment indices supplied to framebuffer.");

	buildBufsList(idx, activeColorTarget);
	use_recursive(idx, activeColorTargets...);

}

#endif