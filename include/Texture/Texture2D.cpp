
#include "./Texture2D.hpp"

GL::Texture2D::Texture2D(unsigned int w, unsigned int h, unsigned int unit, ColorFormat format, DataType type) : Texture(GL_TEXTURE_2D, unit, format, type), w(w), h(h) {
	
	if (w == 0u) throw Exception("Attempt to create a 2D texture with a width of zero.");
	if (h == 0u) throw Exception("Attempt to create a 2D texture with a height of zero.");

	bind();
	glTexImage2D(GL_TEXTURE_2D, 0, GPUStorageType, w, h, 0, GPUFormat, GL_UNSIGNED_BYTE, nullptr);

}

unsigned int GL::Texture2D::width() const { return w; }

unsigned int GL::Texture2D::height() const { return h; }

