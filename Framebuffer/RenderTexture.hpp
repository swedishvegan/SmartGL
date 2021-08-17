#ifndef RENDERTEXTURE_HPP
#define RENDERTEXTURE_HPP

#include "Framebuffer/RenderTarget.hpp"

namespace GL {

	class RenderTexture : public RenderTarget {
	public:

		void bind() const;

		GLuint getID() const;
		
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
		ColorRenderTexture(unsigned int width, unsigned int height, unsigned int unit = 0u, bool multisampled = false, ColorFormat format = ColorFormat::RGBA, DataType type = DataType::F32)
			: RenderTexture(width, height, unit, multisampled, format, type, DepthStencilFormat::DEPTH_16, true) { };
	};

	class DepthStencilRenderTexture : public RenderTexture {
	public:
		DepthStencilRenderTexture(unsigned int width, unsigned int height, unsigned int unit = 0u, bool multisampled = false, DepthStencilFormat format = DepthStencilFormat::DEPTH_32)
			: RenderTexture(width, height, unit, multisampled, ColorFormat::R, DataType::U8, format, false) { };
	};

}

void GL::RenderTexture::bind() const {

	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(target, ID);

}

GLuint GL::RenderTexture::getID() const { return ID; }

void GL::RenderTexture::setWrapMode(TextureWrap mode) const {

	GLint wrapMode;
	if (mode == TextureWrap::REPEAT) wrapMode = GL_REPEAT;
	else if (mode == TextureWrap::MIRRORED_REPEAT) wrapMode = GL_MIRRORED_REPEAT;
	else if (mode == TextureWrap::CLAMP_TO_EDGE) wrapMode = GL_CLAMP_TO_EDGE;
	else throw Exception("Depth-stencil textures are not allowed to be sampled from their border.");

	bind();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

}

void GL::RenderTexture::setMinFilter(TextureFilter mode) const {

	if (isInt && mode == TextureFilter::LINEAR) throw Exception("A render texture with an integer GPU data type cannot use linear filtering.");

	bind();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (mode == TextureFilter::NEAREST) ? GL_NEAREST : GL_LINEAR);

}

void GL::RenderTexture::setMagFilter(TextureFilter mode) const {

	if (isInt && mode == TextureFilter::LINEAR) throw Exception("A render texture with an integer GPU data type cannot use linear filtering.");

	bind();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (mode == TextureFilter::NEAREST) ? GL_NEAREST : GL_LINEAR);

}

GL::RenderTexture::~RenderTexture() { if (ID) glDeleteTextures(1, &ID); }

GL::RenderTexture::RenderTexture(unsigned int width, unsigned int height, unsigned int unit, bool multisampled, ColorFormat colorFormat, DataType colorDataType, DepthStencilFormat depthStencilFormat, bool isColorBuffer) {
	
	if (width == 0u) throw Exception("Render texture width is zero.");
	if (height == 0u) throw Exception("Render texture height is zero.");

	w = width; h = height;
	isTex = true;
	this->unit = unit;
	isMulti = multisampled;
	
	isUint = (
		colorDataType == DataType::U8 ||
		colorDataType == DataType::U16 ||
		colorDataType == DataType::U32
	);
	isInt = (
		isUint ||
		colorDataType == DataType::I8 ||
		colorDataType == DataType::I16 ||
		colorDataType == DataType::I32
	);

	isColor = isColorBuffer;
	hasStencil = (depthStencilFormat == DepthStencilFormat::DEPTH_24_STENCIL_8 || depthStencilFormat == DepthStencilFormat::DEPTH_32_STENCIL_8);

	target = (multisampled) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;

	glGenTextures(1, &ID);
	bind();

	if (!multisampled) {

		setWrapMode(TextureWrap::CLAMP_TO_EDGE);

		GLint filterMode = isInt ? GL_NEAREST : GL_LINEAR;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);

	}

	if (isInt) {

		setMinFilter(TextureFilter::NEAREST);
		setMagFilter(TextureFilter::NEAREST);

	}
	else {

		setMinFilter(TextureFilter::LINEAR);
		setMagFilter(TextureFilter::LINEAR);

	}

	if (isColor) {
		
		bool isInt = !(colorDataType == DataType::F16 || colorDataType == DataType::F32);
		GLenum internalformat = _util::colorStorageTypes[4 * (int)colorDataType + (int)colorFormat];
		if (multisampled) glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, internalformat, w, h, GL_TRUE);
		else glTexImage2D(GL_TEXTURE_2D, 0, internalformat, w, h, 0, _util::formats[(int)colorFormat + 4 * isInt], GL_UNSIGNED_BYTE, nullptr);

	}
	else {

		GLenum internalformat = _util::depthStencilStorageTypes[(int)depthStencilFormat];
		if (multisampled) glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, internalformat, w, h, GL_TRUE);
		else glTexImage2D(GL_TEXTURE_2D, 0, internalformat, w, h, 0, (hasStencil) ? GL_DEPTH_STENCIL : GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);

	}
	
}

#endif