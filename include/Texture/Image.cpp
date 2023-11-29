
#include "./Image.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "./../util/stb_image.h"

GL::Image::Image(const char* filePath, GL::ColorFormat format, unsigned int ccRotation) : inputString(filePath), format(format) {
	
	stbi_set_flip_vertically_on_load(true);
	numComps = (int)format + 1;
	data = stbi_load(filePath, &w, &h, nullptr, numComps);
	rotate(ccRotation);
	errorMessage = (data == nullptr) ? stbi_failure_reason() : nullptr;

}

GL::Image::Image(const unsigned char* compressedData, unsigned int dataSize, GL::ColorFormat format, unsigned int ccRotation, const char* inputString) : inputString(inputString), format(format) {

	stbi_set_flip_vertically_on_load(true);
	numComps = (int)format + 1;
	data = stbi_load_from_memory(compressedData, dataSize, &w, &h, nullptr, numComps);
	rotate(ccRotation);
	errorMessage = (data == nullptr) ? stbi_failure_reason() : nullptr;

}

GL::Image::~Image() {
	
	if (loadSuccess()) {

		if (stbi) stbi_image_free(data);
		else delete[] data;

	}

}

bool GL::Image::loadSuccess() { return (errorMessage == nullptr); }

const char* GL::Image::getError() { return errorMessage; }

const char* GL::Image::getInputString() { return inputString; }

unsigned int GL::Image::getWidth() { return (unsigned int)w; }

unsigned int GL::Image::getHeight() { return (unsigned int)h; }

unsigned char* GL::Image::getData() { return data; }

GL::ColorFormat GL::Image::getFormat() { return format; }

unsigned char& GL::Image::operator () (unsigned int x, unsigned int y, unsigned int colorComponent) {

	x %= (unsigned int)w; y %= (unsigned int)h;
	colorComponent %= numComps;

	return data[numComps * (y * w + x) + colorComponent];

}

void GL::Image::rotate(unsigned int rot) {

	rot %= 4u;
	if (rot == 0u) {

		stbi = true;
		return;

	}

	unsigned char* tempData = new unsigned char[w * h * numComps];

	for (int x = 0; x < w; x++)
		for (int y = 0; y < h; y++) {

			int xNew, yNew;

			if (rot == 1u) {
				xNew = h - 1 - y;
				yNew = x;
			}
			else if (rot == 2u) {
				xNew = w - 1 - x;
				yNew = h - 1 - y;
			}
			else {
				xNew = y;
				yNew = w - 1 - x;
			}

			for (int comp = 0; comp < numComps; comp++) tempData[numComps * (w * yNew + xNew) + comp] = this->operator()(x, y, comp);

		}

	if (rot % 4u == 1u) {

		int temp = w;
		w = h;
		h = temp;

	}

	stbi_image_free(data);
	data = tempData;
	stbi = false;

}