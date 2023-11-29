#ifndef TEXTURECUBEMAP_HPP
#define TEXTURECUBEMAP_HPP

#include "./Texture.hpp"
#include "./Image.hpp"

enum class CubeMapFace { X_POS, X_NEG, Y_POS, Y_NEG, Z_POS, Z_NEG };

namespace GL {

	template <typename S>
	class TextureCubeMap : public CoupledTexture<S> {
	public:

		TextureCubeMap(unsigned int dim, unsigned int unit = 0u, ColorFormat format = ColorFormat::RGBA, DataType type = DataType::F32);

		unsigned int sideLength() const;

		void setFaceData(CubeMapFace face, S* data) const;

	protected:

		unsigned int dim;

	};

	typedef TextureCubeMap<unsigned char> ImageTextureCubeMap;

}

template <typename S>
GL::TextureCubeMap<S>::TextureCubeMap(unsigned int dim, unsigned int unit, ColorFormat format, DataType type) : Texture(GL_TEXTURE_CUBE_MAP, unit, format, type), CoupledTexture<S>(GL_TEXTURE_CUBE_MAP, unit, format, type), dim(dim) {

	if (dim == 0u) throw Exception("Attempt to create a cube map texture with a dimension of zero.");

	Texture::colorFormat = format;

	Texture::bind();
	for (int i = 0; i < 6; i++)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, Texture::GPUStorageType, dim, dim, 0, Texture::GPUFormat, CoupledTexture<S>::CPUStorageType, nullptr);

}

template <typename S>
unsigned int GL::TextureCubeMap<S>::sideLength() const { return dim; }

template <typename S>
void GL::TextureCubeMap<S>::setFaceData(CubeMapFace face, S* data) const {

	if (data == nullptr) throw Exception("Attempt to supply nullptr for cube map face data.");

	Texture::bind();
	glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + (int)face, 0, 0, 0, dim, dim, Texture::GPUFormat, CoupledTexture<S>::CPUStorageType, data);

}

#endif