
#include "./Texture3D.hpp"


GL::Texture3D::Texture3D(unsigned int w, unsigned int h, unsigned int d, unsigned int unit, ColorFormat format, DataType type) : Texture(GL_TEXTURE_3D, unit, format, type), w(w), h(h), d(d) {

	if (w == 0u) throw Exception("Attempt to create a 3D texture with a width of zero.");
	if (h == 0u) throw Exception("Attempt to create a 3D texture with a height of zero.");
	if (d == 0u) throw Exception("Attempt to create a 3D texture with a depth of zero.");

	bind();
	glTexImage3D(GL_TEXTURE_3D, 0, GPUStorageType, w, h, d, 0, GPUFormat, GL_UNSIGNED_BYTE, nullptr);

}

unsigned int GL::Texture3D::width() const { return w; }

unsigned int GL::Texture3D::height() const { return h; }

unsigned int GL::Texture3D::depth() const { return d; }