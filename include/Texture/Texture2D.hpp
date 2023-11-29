#ifndef TEXTURE2D_HPP
#define TEXTURE2D_HPP

#include <cstring>

#include "./Texture.hpp"
#include "./Image.hpp"

namespace GL {

	class Texture2D : virtual public Texture {
	public:

		Texture2D(unsigned int w, unsigned int h, unsigned int unit = 0u, ColorFormat format = ColorFormat::RGBA, DataType type = DataType::F32);

		unsigned int width() const;

		unsigned int height() const;

	protected:

		unsigned int w, h;

	};

	template <typename S>
	class CoupledTexture2D : public CoupledTexture<S>, public Texture2D {
	public:

		CoupledTexture2D(unsigned int w, unsigned int h, unsigned int unit = 0u, ColorFormat format = ColorFormat::RGBA, DataType type = DataType::F32);
		CoupledTexture2D(unsigned int w, unsigned int h, S* data, bool memcopy, unsigned int unit = 0u, ColorFormat format = ColorFormat::RGBA, DataType type = DataType::F32);

		S getPixel(unsigned int x, unsigned int y, unsigned int colorComponent) const;
		void setPixel(unsigned int x, unsigned int y, unsigned int colorComponent, S value);

		S& operator () (unsigned int x, unsigned int y, unsigned int colorComponent);

		void writeToGPU();
		void readFromGPU();

		~CoupledTexture2D();

	protected:

		S* data = nullptr;
		bool ownsData;
		unsigned int s, e;
		bool first = true;

		void init(S* data, bool memcopy);

		unsigned int getIndex(unsigned int x, unsigned int y);

	};

	class ImageTexture : public CoupledTexture2D<unsigned char> {
	public:
		ImageTexture(Image& image, bool memcopy, unsigned int unit = 0u, ColorFormat format = ColorFormat::RGBA, DataType type = DataType::F32) : Texture(GL_TEXTURE_2D, unit, format, type), CoupledTexture2D<unsigned char>(image.getWidth(), image.getHeight(), image.getData(), memcopy, unit, format, type) { }
	};

}

template <typename S>
GL::CoupledTexture2D<S>::CoupledTexture2D(unsigned int w, unsigned int h, unsigned int unit, ColorFormat format, DataType type) : Texture(GL_TEXTURE_2D, unit, format, type), CoupledTexture<S>(GL_TEXTURE_2D, unit, format, type), Texture2D(w, h, unit, format, type) { init(nullptr, false); }

template <typename S>
GL::CoupledTexture2D<S>::CoupledTexture2D(unsigned int w, unsigned int h, S* data, bool memcopy, unsigned int unit, ColorFormat format, DataType type) : Texture(GL_TEXTURE_2D, unit, format, type), CoupledTexture<S>(GL_TEXTURE_2D, unit, format, type), Texture2D(w, h, unit, format, type) { init(data, memcopy); }

template <typename S>
S GL::CoupledTexture2D<S>::getPixel(unsigned int x, unsigned int y, unsigned int colorComponent) const {

	if (colorComponent >= numComps) throw Exception("Color component passed to CoupledTexture2D::getPixel() was " + std::to_string(colorComponent) + ", but the maximum allowed value is " + std::to_string(numComps - 1u) + ".");
	x %= w; y %= h;

	return data[getIndex(x, y) + colorComponent];

}

template <typename S>
void GL::CoupledTexture2D<S>::setPixel(unsigned int x, unsigned int y, unsigned int colorComponent, S value) { this->operator()(x, y, colorComponent) = value; }

template <typename S>
S& GL::CoupledTexture2D<S>::operator () (unsigned int x, unsigned int y, unsigned int colorComponent) {

	if (colorComponent >= numComps) throw Exception("Color component passed to CoupledTexture2D::getPixel() was " + std::to_string(colorComponent) + ", but the maximum allowed value is " + std::to_string(numComps - 1u) + ".");
	x %= w; y %= h;

	if (first) {

		s = y; e = y;
		first = false;

	}
	else {

		if (y < s) s = y;
		if (y > e) e = y;

	}

	return data[getIndex(x, y) + colorComponent];

}

template <typename S>
void GL::CoupledTexture2D<S>::writeToGPU() {

	if (first) return;

	bind();
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, s, w, 1u + e - s, GPUFormat, CoupledTexture<S>::CPUStorageType, (void*)(data + getIndex(0u, s)));

	first = true;

}

template <typename S>
void GL::CoupledTexture2D<S>::readFromGPU() {

	bind();
	glGetTexImage(GL_TEXTURE_2D, 0, GPUFormat, CoupledTexture<S>::CPUStorageType, data);

	first = true;

}

template <typename S>
GL::CoupledTexture2D<S>::~CoupledTexture2D() { if (data && ownsData) delete[] data; }

template <typename S>
void GL::CoupledTexture2D<S>::init(S* data, bool memcopy) {
	
	unsigned int length = w * h * numComps;

	if (data) {

		if (memcopy) {

			this->data = new S[length];
			std::memcpy(
				this->data,
				data,
				length * sizeof(S)
			);

			ownsData = true;

		}
		else {

			this->data = data;
			ownsData = false;

		}

	}
	else {

		this->data = new S[length];
		ownsData = true;

	}

	if (data) {

		bind();
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GPUFormat, CoupledTexture<S>::CPUStorageType, data);

	}

}

template <typename S>
unsigned int GL::CoupledTexture2D<S>::getIndex(unsigned int x, unsigned int y) { return numComps * (w * y + x); }

#endif