#ifndef FONTNORMALIZER_HPP
#define FONTNORMALIZER_HPP

#define GLEW_STATIC
#include <GL/glew.h>

#define SmartGL_NO_PHYSICS
#include <SmartGL.hpp>

#include "Font/FontLoader.hpp"
#include "util/Sorter_implementations.hpp"

class FontNormalizer {
public:

	FontNormalizer(GL::FontLoader& loader, int dim, uint unit);

	void bind();

	int getDim();

	uint getUnit();

	uchar getGlyphEntry(uchar c, GL::ivec2 coords);
	
	uchar getRankedGlyphEntry(uint rank, GL::ivec2 coords);

	float getRankedFrequency(uint rank);

	GL::vec2 getFrequencyBounds();

	~FontNormalizer();

private:

	uchar* data;
	uchar* rankedData;

	float minFreq, maxFreq;

	struct tuple { float freq; uint idx; bool operator < (tuple t) { return freq > t.freq; } bool operator > (tuple t) { return freq < t.freq; } };
	QuickSorter<tuple>* sorter;

	GLuint tex;
	int d;
	uint u;

	uchar sample(GL::FontLoader& loader, uchar c, GL::vec2 coords);

};

FontNormalizer::FontNormalizer(GL::FontLoader& loader, int dim, uint unit) : u(unit) {

	if (dim < 1) dim = 1; d = dim;
	data = new uchar[dim * dim * 256];

	for (uint c = 0; c < 256; c++)
		for (int x = 0; x < dim; x++)
			for (int y = 0; y < dim; y++) {

				GL::vec2 normCoords = (GL::vec2(x, y) + 0.5f) / GL::vec2(dim, dim);
				uchar sampleChar = sample(loader, c, normCoords * GL::vec2(loader.getGlyphInfo((uchar)c).size - 1));
				
				int idx = (256 * dim) * (dim - 1 - y) + dim * c + x;
				data[idx] = sampleChar;

			}

	tuple* tuples = new tuple[256];
	for (uint c = 0; c < 256; c++) {

		float freq = 0.0f;
		for (int x = 0; x < dim; x++) for (int y = 0; y < dim; y++) {

			uint entry = getGlyphEntry(c, GL::ivec2(x, y));
			freq += (float)entry / 255.0f;

		}
		freq /= (float)(dim * dim);

		tuples[c] = tuple{ freq, c };

	}

	sorter = new QuickSorter<tuple>(tuples, 256);
	rankedData = new uchar[dim * dim * 256];

	for (uint c = 0; c < 256; c++) {

		uint cSrc = sorter->operator[](c).idx;

		for (int x = 0; x < dim; x++) for (int y = 0; y < dim; y++) {

			int idx = (256 * dim) * y + dim * c + x;
			rankedData[idx] = getGlyphEntry(cSrc, GL::ivec2(x, y));

		}

	}

	bool isValid[256];

	for (uint c = 0; c < 256; c++) {

		GL::ivec2 ss = loader.getGlyphInfo(c).size;
		int s = ss.x * ss.y;

		isValid[c] = (s > 0);

	}

	minFreq = 0.0; maxFreq = 1.0;
	bool minFound = false; bool maxFound = false;

	for (uint c = 0; c < 256; c++) {
		
		float mif = sorter->operator[](255 - c).freq;
		float maf = sorter->operator[](c).freq;

		if (isValid[sorter->operator[](255 - c).idx] && !minFound) { minFreq = mif; minFound = true; }
		if (!maxFound) { maxFreq = maf; maxFound = true; }

		if (minFound && maxFound) break;

	}
	
	glGenTextures(1, &tex);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, dim * 256, dim, 0, GL_RED, GL_UNSIGNED_BYTE, rankedData);

}

void FontNormalizer::bind() {

	glActiveTexture(GL_TEXTURE0 + u);
	glBindTexture(GL_TEXTURE_2D, tex);

}

int FontNormalizer::getDim() { return d; }

uint FontNormalizer::getUnit() { return u; }

uchar FontNormalizer::getGlyphEntry(uchar c, GL::ivec2 coords) {

	coords = clamp(coords, GL::ivec2(), GL::ivec2(d) - 1);
	int idx = coords.y * (256 * d) + d * (int)c + coords.x;
	return data[idx];

}

uchar FontNormalizer::getRankedGlyphEntry(uint rank, GL::ivec2 coords) {

	rank %= 256;
	coords = clamp(coords, GL::ivec2(), GL::ivec2(d) - 1);
	int idx = coords.y * (256 * d) + d * rank + coords.x;
	return rankedData[idx];

}

float FontNormalizer::getRankedFrequency(uint rank) {

	rank %= 256;
	return sorter->operator[](rank).freq;

}

GL::vec2 FontNormalizer::getFrequencyBounds() { return GL::vec2(minFreq, maxFreq); }

FontNormalizer::~FontNormalizer() { delete[] data; delete[] rankedData; delete sorter; glDeleteTextures(1, &tex); }

uchar FontNormalizer::sample(GL::FontLoader& loader, uchar c, GL::vec2 coords) {
	
	GL::ivec2 co00 = floor(coords);
	GL::ivec2 co11 = co00 + 1;
	GL::ivec2 co01(co00.x, co11.y);
	GL::ivec2 co10(co11.x, co00.y);

	uchar cu00 = loader.getGlyphEntry(c, co00);
	uchar cu10 = loader.getGlyphEntry(c, co10);
	uchar cu01 = loader.getGlyphEntry(c, co01);
	uchar cu11 = loader.getGlyphEntry(c, co11);

	float c00 = (float)((uint)cu00);
	float c10 = (float)((uint)cu10);
	float c01 = (float)((uint)cu01);
	float c11 = (float)((uint)cu11);

	GL::vec2 f = fract(coords);
	float c0 = GL::mix(c00, c10, f.x);
	float c1 = GL::mix(c01, c11, f.x);
	float cf = GL::mix(c0, c1, f.y);

	return (uchar)((uint)cf);

}

#endif