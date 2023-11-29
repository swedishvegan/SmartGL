
#include "./Texture.hpp"

void GL::Texture::bind() const {

	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(target, ID);

}

void GL::Texture::bindToTextureUnit(unsigned int textureUnit) const {

	if (textureUnit >= _util::maxTextureUnits) throw Exception("Attempt to bind texture to the texture unit " + std::to_string(textureUnit) + ", but the maximum texture unit allowed is " + std::to_string(_util::maxTextureUnits - 1) + ".");

	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(target, ID);

}

void GL::Texture::bindToImageUnit(unsigned int imageUnit, AccessType accessType) const {

	if (imageUnit >= _util::maxImageUnits) throw Exception("Attempt to bind texture to the image unit " + std::to_string(imageUnit) + ", but the maximum image unit allowed is " + std::to_string(_util::maxImageUnits - 1) + ".");
	
	int iColorFormat = (int)colorFormat;
	if (iColorFormat == 2) throw Exception("A texture accessed in a program as an image cannot have an RGB format.");
	static GLenum accessTypes[] = { GL_READ_ONLY, GL_WRITE_ONLY, GL_READ_WRITE };
	GLboolean layered = (target == GL_TEXTURE_CUBE_MAP || target == GL_TEXTURE_3D) ? GL_TRUE : GL_FALSE;

	glBindImageTexture(imageUnit, ID, 0, layered, 0, accessTypes[(int)accessType], _util::colorStorageTypes[4 * (int)GPUDataType + iColorFormat]);

}

GLuint GL::Texture::getID() const { return ID; }

unsigned int GL::Texture::getUnit() const { return unit; }

GL::ColorFormat GL::Texture::getColorFormat() const { return colorFormat; }

GL::DataType GL::Texture::getInternalDataType() const { return GPUDataType; }

void GL::Texture::setWrapMode(TextureWrap mode) const {
	
	GLint wrapMode = _util::wrapModes[(int)mode];

	bind();
	glTexParameteri(target, GL_TEXTURE_WRAP_S, wrapMode);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, wrapMode);
	glTexParameteri(target, GL_TEXTURE_WRAP_R, wrapMode);

}

void GL::Texture::setMinFilter(TextureFilter mode) const {

	if (isInt && mode == TextureFilter::LINEAR) throw Exception("A texture with an integer GPU data type cannot use linear filtering.");

	bind();
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, (mode == TextureFilter::NEAREST) ? GL_NEAREST : GL_LINEAR);

}

void GL::Texture::setMinFilter(TextureFilter mode, TextureFilter mipmapMode) const {

	if (!hasMipmaps) throw Exception("A texture min filter cannot use mipmaps if no mipmaps have been supplied.");
	if (isInt && (mode == TextureFilter::LINEAR || mipmapMode == TextureFilter::LINEAR)) throw Exception("A texture with an integer GPU data type cannot use linear filtering.");

	bind();
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, getFilterMode(mode, mipmapMode));

}

void GL::Texture::setMagFilter(TextureFilter mode) const {

	if (isInt && mode == TextureFilter::LINEAR) throw Exception("A texture with an integer GPU data type cannot use linear filtering.");

	bind();
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, (mode == TextureFilter::NEAREST) ? GL_NEAREST : GL_LINEAR);

}

void GL::Texture::updateMipmaps() {
	
	bind();
	glGenerateMipmap(target);
	hasMipmaps = true;

}

GL::Texture::~Texture() { if (ID) glDeleteTextures(1, &ID); }

GL::Texture::Texture(GLenum target, unsigned int unit, ColorFormat format, DataType type) {

	if (unit >= _util::maxTextureUnits) throw Exception("Texture unit " + std::to_string(unit) + " is too high.");

	this->target = target;
	this->unit = unit;
	GPUStorageType = _util::colorStorageTypes[4 * (int)type + (int)format];
	GPUDataType = type;
	colorFormat = format;
	isInt = !(type == DataType::F16 || type == DataType::F32);
	numComps = (unsigned int)format + 1u;
	GPUFormat = _util::formats[(int)format + 4 * isInt];

	glGenTextures(1, &ID);
	bind();
	setWrapMode(TextureWrap::CLAMP_TO_EDGE);
	TextureFilter filterMode = (isInt) ? TextureFilter::NEAREST : TextureFilter::LINEAR;
	setMinFilter(filterMode);
	setMagFilter(filterMode);

}

GLenum GL::Texture::getFilterMode(TextureFilter mode, TextureFilter mipmapMode) {

	static const GLenum filterEnums[] = { GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR };
	return filterEnums[2 * (int)mode + (int)mipmapMode];

}