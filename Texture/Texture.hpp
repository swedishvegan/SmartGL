#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <type_traits>

#include "util/enums.hpp"
#include "util/util.hpp"

namespace GL {

	class Texture : public _util {
	public:

		void bind() const;

		void bindToImageUnit(unsigned int imageUnit, AccessType accessType = AccessType::READ_WRITE) const;

		GLuint getID() const;

		unsigned int getUnit() const;

		ColorFormat getColorFormat() const;

		DataType getInternalDataType() const;

		template <typename T>
		void setBorderColor(T R, T G = (T)0, T B = (T)0, T A = (T)0) const;

		void setWrapMode(TextureWrap mode) const;

		void setMinFilter(TextureFilter mode) const;

		void setMinFilter(TextureFilter mode, TextureFilter mipmapMode) const;

		void setMagFilter(TextureFilter mode) const;

		void updateMipmaps();

		~Texture();

	protected:

		GLuint ID;
		GLenum target;
		GLenum GPUStorageType;
		GLenum GPUFormat;
		DataType GPUDataType;
		ColorFormat colorFormat;
		unsigned int numComps;
		unsigned int unit;
		bool isInt;
		bool hasMipmaps = false;
		
		Texture(GLenum target, unsigned int unit, ColorFormat format, DataType type);

		static GLenum getFilterMode(TextureFilter mode, TextureFilter mipmapMode);

	};

	template <typename S>
	class CoupledTexture : virtual public Texture {
	protected:

		DataType CPUDataType;
		GLenum CPUStorageType;

		CoupledTexture(GLenum target, unsigned int unit, ColorFormat format, DataType GPUType);

	};

}

void GL::Texture::bind() const {

	glActiveTexture(GL_TEXTURE0 + unit);
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

template <typename T>
void GL::Texture::setBorderColor(T R, T G, T B, T A) const {

	static GLint iparams[4];
	static GLuint uparams[4];
	static GLfloat fparams[4];

	if (isInt) {

		if (std::is_same<T, GLint>::value) {

			iparams[0] = R;
			iparams[1] = G;
			iparams[2] = B;
			iparams[3] = A;
			glTexParameterIiv(target, GL_TEXTURE_BORDER_COLOR, iparams);

		}
		else if (std::is_same<T, GLuint>::value) {

			uparams[0] = R;
			uparams[1] = G;
			uparams[2] = B;
			uparams[3] = A;
			glTexParameterIuiv(target, GL_TEXTURE_BORDER_COLOR, uparams);

		}
		else throw Exception("Attempt to set integer texture border color with an invalid data type.");

	}
	else {

		if (std::is_same<T, GLint>::value) {

			iparams[0] = R;
			iparams[1] = G;
			iparams[2] = B;
			iparams[3] = A;
			glTexParameteriv(target, GL_TEXTURE_BORDER_COLOR, iparams);

		}
		else if (std::is_same<T, GLfloat>::value) {

			fparams[0] = R;
			fparams[1] = G;
			fparams[2] = B;
			fparams[3] = A;
			glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, fparams);

		}
		else throw Exception("Attempt to set floating-point texture border color with an invalid data type.");

	}

}

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

template <typename S>
GL::CoupledTexture<S>::CoupledTexture(GLenum target, unsigned int unit, ColorFormat format, DataType GPUType) : Texture(target, unit, format, GPUType) {

	if (std::is_same<S, GLfloat>::value) CPUDataType = DataType::F32;
	else if (std::is_same<S, GLbyte>::value) CPUDataType = DataType::I8;
	else if (std::is_same<S, GLshort>::value) CPUDataType = DataType::I16;
	else if (std::is_same<S, GLint>::value) CPUDataType = DataType::I32;
	else if (std::is_same<S, GLubyte>::value) CPUDataType = DataType::U8;
	else if (std::is_same<S, GLushort>::value) CPUDataType = DataType::U16;
	else if (std::is_same<S, GLuint>::value) CPUDataType = DataType::U32;
	else throw Exception("Invalid CPU data type for coupled texture.");

	if (CPUDataType == DataType::F32 && isInt) throw Exception("A coupled texture with an integer GPU data type cannot have a floating point CPU data type.");
	
	CPUStorageType = _util::types[(int)CPUDataType - 1];

}

#endif