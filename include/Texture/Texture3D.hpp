#ifndef TEXTURE3D_HPP
#define TEXTURE3D_HPP

#include "./Texture.hpp"

namespace GL {

	class Texture3D : virtual public Texture {
	public:

		Texture3D(unsigned int w, unsigned int h, unsigned int d, unsigned int unit = 0u, ColorFormat format = ColorFormat::RGBA, DataType type = DataType::F32);

		unsigned int width() const;

		unsigned int height() const;
		
		unsigned int depth() const;

	protected:

		unsigned int w, h, d;

	};

	template <typename S>
	class CoupledTexture3D : public CoupledTexture<S>, public Texture3D {
	public:

		CoupledTexture3D(unsigned int w, unsigned int h, unsigned int d, unsigned int unit = 0u, ColorFormat format = ColorFormat::RGBA, DataType type = DataType::F32);
		CoupledTexture3D(unsigned int w, unsigned int h, unsigned int d, S* data, bool memcopy, unsigned int unit = 0u, ColorFormat format = ColorFormat::RGBA, DataType type = DataType::F32);

		S getPixel(unsigned int x, unsigned int y, unsigned int z, unsigned int colorComponent) const;
		void setPixel(unsigned int x, unsigned int y, unsigned int z, unsigned int colorComponent, S value);

		S& operator () (unsigned int x, unsigned int y, unsigned int z, unsigned int colorComponent);

		void writeToGPU();
		void readFromGPU();

		~CoupledTexture3D();

	protected:

		S* data = nullptr;
		bool ownsData;
		unsigned int s, e;
		bool first = true;

		void init(S* data, bool memcopy);

		unsigned int getIndex(unsigned int x, unsigned int y, unsigned int z);

	};

}

template <typename S>
GL::CoupledTexture3D<S>::CoupledTexture3D(unsigned int w, unsigned int h, unsigned int d, unsigned int unit, ColorFormat format, DataType type) : Texture(GL_TEXTURE_3D, unit, format, type), CoupledTexture<S>(GL_TEXTURE_3D, unit, format, type), Texture3D(w, h, d, unit, format, type) { init(nullptr, false); }

template <typename S>
GL::CoupledTexture3D<S>::CoupledTexture3D(unsigned int w, unsigned int h, unsigned int d, S* data, bool memcopy, unsigned int unit, ColorFormat format, DataType type) : Texture(GL_TEXTURE_3D, unit, format, type), CoupledTexture<S>(GL_TEXTURE_3D, unit, format, type), Texture3D(w, h, d, unit, format, type) { init(data, memcopy); }

template <typename S>
S GL::CoupledTexture3D<S>::getPixel(unsigned int x, unsigned int y, unsigned int z, unsigned int colorComponent) const {

	if (colorComponent >= numComps) throw Exception("Color component passed to CoupledTexture2D::getPixel() was " + std::to_string(colorComponent) + ", but the maximum allowed value is " + std::to_string(numComps - 1u) + ".");
	x %= w; y %= h; z %= d;

	return data[getIndex(x, y, z) + colorComponent];

}

template <typename S>
void GL::CoupledTexture3D<S>::setPixel(unsigned int x, unsigned int y, unsigned int z, unsigned int colorComponent, S value) { this->operator() (x, y, z, colorComponent) = value; }

template <typename S>
S& GL::CoupledTexture3D<S>::operator () (unsigned int x, unsigned int y, unsigned int z, unsigned int colorComponent) {

	if (colorComponent >= numComps) throw Exception("Color component used to set pixel in 3D texture is invalid.");
	x %= w; y %= h; z %= d;

	if (first) {

		s = z; e = z;
		first = false;

	}
	else {

		if (z < s) s = z;
		if (z > e) e = z;

	}

	return data[getIndex(x, y, z) + colorComponent];

}

template <typename S>
void GL::CoupledTexture3D<S>::writeToGPU() {

	if (first) return;

	bind();
	glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, s, w, h, 1u + (e - s), GPUFormat, CoupledTexture<S>::CPUStorageType, (void*)(data + getIndex(0u, 0u, s)));

	first = true;

}

template <typename S>
void GL::CoupledTexture3D<S>::readFromGPU() {

	bind();
	glGetTexImage(GL_TEXTURE_3D, 0, GPUFormat, CoupledTexture<S>::CPUStorageType, data);

	first = true;

}

template <typename S>
GL::CoupledTexture3D<S>::~CoupledTexture3D() { if (data && ownsData) delete[] data; }

template <typename S>
void GL::CoupledTexture3D<S>::init(S* data, bool memcopy) {

	unsigned int length = w * h * d * numComps;

	if (data) {

		if (memcopy) {

			this->data = new S[length];
			memcpy(
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
		glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, w, h, d, GPUFormat, CoupledTexture<S>::CPUStorageType, data);

	}

}

template <typename S>
unsigned int GL::CoupledTexture3D<S>::getIndex(unsigned int x, unsigned int y, unsigned int z) { return numComps * (w * (h * z + y) + x); }

#endif