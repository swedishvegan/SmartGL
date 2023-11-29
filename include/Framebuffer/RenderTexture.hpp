#ifndef RENDERTEXTURE_HPP
#define RENDERTEXTURE_HPP

#include "./RenderTarget.hpp"
#include "./../Texture/TextureBase.hpp"

namespace GL {

	class RenderTexture : public RenderTarget, public TextureBase {
	public:

		void bind() const;

		void bindToTextureUnit(unsigned int textureUnit) const;

		GLuint getID() const;

		unsigned int getUnit() const;
		
		void setWrapMode(TextureWrap mode) const;

		void setMinFilter(TextureFilter mode) const;
		void setMagFilter(TextureFilter mode) const;
		
		~RenderTexture();

	protected:
		
		GLuint ID;
		GLenum target;
		unsigned int unit;
		bool isUint, isInt;

		RenderTexture(unsigned int width, unsigned int height, unsigned int unit, bool multisampled, ColorFormat colorFormat, DataType colorDataType, DepthStencilFormat depthStencilFormat, bool isColorBuffer);

	};

	class ColorRenderTexture : public RenderTexture {
	public:

		ColorRenderTexture(unsigned int width, unsigned int height, unsigned int unit = 0u, bool multisampled = false, ColorFormat format = ColorFormat::RGBA, DataType type = DataType::F32);

		ColorFormat getColorFormat() const;

		DataType getInternalDataType() const;

	private:

		ColorFormat format;
		DataType dataType;

	};

	class DepthStencilRenderTexture : public RenderTexture {
	public:

		DepthStencilRenderTexture(unsigned int width, unsigned int height, unsigned int unit = 0u, bool multisampled = false, DepthStencilFormat format = DepthStencilFormat::DEPTH_32);

		DepthStencilFormat getDepthStencilFormat() const;

	private:

		DepthStencilFormat format;

	};

}

#endif