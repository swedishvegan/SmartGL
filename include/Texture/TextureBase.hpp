#ifndef TEXTUREBASE_HPP
#define TEXTUREBASE_HPP

#include "./../util/util.hpp"
#include "./../util/enums.hpp"

namespace GL {

	class TextureBase : public _util {
	public:

		virtual void bind() const = 0;

		virtual void bindToTextureUnit(unsigned int textureUnit) const = 0;

		virtual GLuint getID() const = 0;

		virtual unsigned int getUnit() const = 0;

		virtual void setWrapMode(TextureWrap mode) const = 0;

		virtual void setMinFilter(TextureFilter mode) const = 0;
		virtual void setMagFilter(TextureFilter mode) const = 0;

	};

}

#endif