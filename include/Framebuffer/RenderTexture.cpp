
#include "./RenderTexture.hpp"

GL::ColorRenderTexture::ColorRenderTexture(unsigned int width, unsigned int height, unsigned int unit, bool multisampled, ColorFormat format, DataType type)
    : RenderTexture(width, height, unit, multisampled, format, type, DepthStencilFormat::DEPTH_16, true), format(format), dataType(type) { };

GL::ColorFormat GL::ColorRenderTexture::getColorFormat() const { return format; }

GL::DataType GL::ColorRenderTexture::getInternalDataType() const { return dataType; }

GL::DepthStencilRenderTexture::DepthStencilRenderTexture(unsigned int width, unsigned int height, unsigned int unit, bool multisampled, DepthStencilFormat format)
    : RenderTexture(width, height, unit, multisampled, ColorFormat::R, DataType::U8, format, false), format(format) { };

GL::DepthStencilFormat GL::DepthStencilRenderTexture::getDepthStencilFormat() const { return format; }

void GL::RenderTexture::bind() const {

	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(target, ID);

}

void GL::RenderTexture::bindToTextureUnit(unsigned int textureUnit) const {

	if (textureUnit >= _util::maxTextureUnits) throw Exception("Attempt to bind render texture to the texture unit " + std::to_string(textureUnit) + ", but the maximum texture unit allowed is " + std::to_string(_util::maxTextureUnits - 1) + ".");

	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(target, ID);

}

GLuint GL::RenderTexture::getID() const { return ID; }

unsigned int GL::RenderTexture::getUnit() const { return unit; }

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
	
	if (isColorBuffer) {

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

	}
	else { isInt = false; isUint = false; }

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