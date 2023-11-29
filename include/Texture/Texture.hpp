#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <type_traits>

#include "./../util/enums.hpp"
#include "./../util/util.hpp"
#include "./TextureBase.hpp"

namespace GL {

	class Texture : public TextureBase {
	public:

		void bind() const;

		void bindToTextureUnit(unsigned int textureUnit) const;

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