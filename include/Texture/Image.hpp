#ifndef IMAGE_HPP
#define IMAGE_HPP

#include "./../util/enums.hpp"

namespace GL {

	class Image {
	public:

		Image(const char* filePath, GL::ColorFormat format = GL::ColorFormat::RGBA, unsigned int ccRotation = 0u);

		Image(const unsigned char* compressedData, unsigned int dataSize, GL::ColorFormat format = GL::ColorFormat::RGBA, unsigned int ccRotation = 0u, const char* inputString = nullptr);

		~Image();

		bool loadSuccess();

		const char* getError();

		const char* getInputString();

		unsigned int getWidth();
		unsigned int getHeight();
		unsigned char* getData();
		GL::ColorFormat getFormat();

		unsigned char& operator () (unsigned int x, unsigned int y, unsigned int colorComponent);

	private:

		unsigned char* data;
		bool stbi;
		const char* inputString;
		const char* errorMessage;
		int w, h;
		GL::ColorFormat format;
		int numComps;

		void rotate(unsigned int rot);

	};

}

#endif