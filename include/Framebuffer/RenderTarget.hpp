#ifndef RENDERTARGET_HPP
#define RENDERTARGET_HPP

#include "./../util/util.hpp"
#include "./../util/enums.hpp"

namespace GL {

	class RenderTarget : public _util {
	public:

		bool isTexture() const;

		bool isColorTarget() const;
		bool isDepthTarget() const;
		bool isStencilTarget() const;

		bool isMultisampled() const;

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

#endif