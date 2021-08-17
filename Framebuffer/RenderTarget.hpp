#ifndef RENDERTARGET_HPP
#define RENDERTARGET_HPP

#include "util/util.hpp"
#include "util/enums.hpp"

namespace GL {

	class RenderTarget : public _util {
	public:
		
		virtual void bind() const = 0;

		bool isTexture() const;

		bool isColorTarget() const;
		bool isDepthTarget() const;
		bool isStencilTarget() const;

		bool isMultisampled() const;

		virtual GLuint getID() const = 0;

		unsigned int width() const;
		unsigned int height() const;

	protected:
		
		unsigned int w = 0u, h = 0u;
		bool isTex;
		bool isColor;
		bool hasStencil;
		bool isMulti;

	};

}

bool GL::RenderTarget::isTexture() const { return isTex; }

bool GL::RenderTarget::isColorTarget() const { return isColor; }

bool GL::RenderTarget::isDepthTarget() const { return !isColor; }

bool GL::RenderTarget::isStencilTarget() const { return !isColor && hasStencil; }

bool GL::RenderTarget::isMultisampled() const { return isMulti; }

unsigned int GL::RenderTarget::width() const { return w; }

unsigned int GL::RenderTarget::height() const { return h; }

#endif