#ifndef FONTCONVERTER_HPP
#define FONTCONVERTER_HPP

#include "FontNormalizer.hpp"

class FontConverter {
public:

	FontConverter(GL::FontLoader& loader, GL::uvec2 outputSize, uint bitmapResolution, uint letterSize);

	FontConverter(GL::FontLoader& loader, uint bitmapResolution, uint letterSize);

	struct Settings {

		bool isColored;
		bool normalizeColors;
		float correctiveExponent;

		Settings() : isColored(true), normalizeColors(false), correctiveExponent(1.0) { }

	};

	template <typename ColorTexture>
	void convert(ColorTexture& tex, GL::Framebuffer& outputFramebuffer, Settings settings = Settings{ });

	template <typename ColorTexture>
	void convert(ColorTexture& tex, Settings settings = Settings{ });

private:

	GL::uvec2 outputSize;
	GL::uvec2 miniSize;

	GL::Framebuffer miniFramebuffer;
	GL::ColorRenderTexture miniRenderTexture;

	GL::FontLoader& loader;
	FontNormalizer normalizer;

	static GL::Program* minimize;
	static GL::Program* maximize;
	static GL::UniformTable* minimizeUnis;
	static GL::UniformTable* maximizeUnis;
	static GL::UniformBufferTable* frequencies;
	static FontConverter* lastUsed;

	static const char* commonVertShader;
	static const char* minimizeFragShader;
	static const char* maximizeFragShader;

	void constructor();

	template <typename ColorTexture>
	void convert(ColorTexture& tex, GL::Framebuffer* outputFb, Settings settings);

	static void loadShaders();

};

GL::Program* FontConverter::minimize = nullptr;
GL::Program* FontConverter::maximize = nullptr;
GL::UniformTable* FontConverter::minimizeUnis = nullptr;
GL::UniformTable* FontConverter::maximizeUnis = nullptr;
GL::UniformBufferTable* FontConverter::frequencies = nullptr;
FontConverter* FontConverter::lastUsed = nullptr;

#define _FontConverter_initializers(theScreenSize) \
\
miniFramebuffer(miniSize.x, miniSize.y), \
miniRenderTexture(miniSize.x, miniSize.y, 1, false, GL::ColorFormat::RGB, GL::DataType::F32), \
loader(loader), \
normalizer(loader, bitmapResolution, 0), \
outputSize(theScreenSize), \
miniSize(outputSize / letterSize)

FontConverter::FontConverter(GL::FontLoader& loader, GL::uvec2 screenSize, uint bitmapResolution, uint letterSize) : _FontConverter_initializers(screenSize) { constructor(); }

FontConverter::FontConverter(GL::FontLoader& loader, uint bitmapResolution, uint letterSize) : _FontConverter_initializers(GL::getScreenSize()) { constructor(); }

template <typename ColorTexture>
void FontConverter::convert(ColorTexture& tex, GL::Framebuffer& outputFramebuffer, FontConverter::Settings settings) { convert(tex, &outputFramebuffer, settings); }

template <typename ColorTexture>
void FontConverter::convert(ColorTexture& tex, FontConverter::Settings settings) { convert(tex, nullptr, settings); }

void FontConverter::constructor() {

	loadShaders();

	miniRenderTexture.setMinFilter(GL::TextureFilter::NEAREST);
	miniRenderTexture.setMagFilter(GL::TextureFilter::NEAREST);
	miniFramebuffer.setColorTarget(miniRenderTexture);

}

template <typename ColorTexture>
void FontConverter::convert(ColorTexture& tex, GL::Framebuffer* outputFb, FontConverter::Settings settings) {

	if (!(
		tex.getInternalDataType() == GL::DataType::F16 ||
		tex.getInternalDataType() == GL::DataType::F32
	)) throw GL::Exception("FontConverter's input texture must have floating point internal data type.");

	if (tex.width() != outputSize.x || tex.height() != outputSize.y) throw GL::Exception("Input color texture's dimensions must match FontConverter's dimensions.");
	if (outputFb) if (tex.width() != outputFb->getWidth() || tex.height() != outputFb->getHeight()) throw GL::Exception("FontConverter input color texture's dimensions must match output framebuffer's dimensions.");
	
	if (lastUsed != this) {

		for (int i = 0; i < 256; i++) frequencies->setElement<float>("freqs", i, normalizer.getRankedFrequency(255 - i));
		frequencies->update();

	}
	lastUsed = this;

	minimizeUnis->set<int>("bigScreen", tex.getUnit());
	minimizeUnis->update();

	minimize->use();
	miniFramebuffer.use();
	tex.bind();
	GL::Render::drawDefault();

	maximizeUnis->set<int>("letterMap", normalizer.getUnit());
	maximizeUnis->set<GL::ivec2>("numDivisions", miniSize);
	maximizeUnis->set<GL::vec2>("freqBounds", (settings.normalizeColors) ? normalizer.getFrequencyBounds() : GL::vec2(0.0f, 1.0f));
	maximizeUnis->set<int>("isColored", settings.isColored);
	maximizeUnis->set<float>("correctiveExponent", settings.correctiveExponent);
	maximizeUnis->set<int>("smallScreen", miniRenderTexture.getUnit());
	maximizeUnis->update();

	maximize->use();
	if (outputFb) outputFb->use();
	else GL::Framebuffer::useDefault(outputSize.x, outputSize.y);
	frequencies->bind();
	miniRenderTexture.bind();
	normalizer.bind();
	GL::Render::drawDefault();

}

void FontConverter::loadShaders() {

	if (minimize) return;
	
	GL::ShaderLoader vert(GL::ShaderType::VERTEX);
	vert.init(commonVertShader, false);

	GL::ShaderLoader miniFrag(GL::ShaderType::FRAGMENT);
	miniFrag.init(minimizeFragShader, false);

	GL::ShaderLoader maxiFrag(GL::ShaderType::FRAGMENT);
	maxiFrag.init(maximizeFragShader, false);

	minimize = new GL::Program();
	minimize->init(vert, miniFrag);

	maximize = new GL::Program();
	maximize->init(vert, maxiFrag);

	minimizeUnis = new GL::UniformTable(*minimize);
	minimizeUnis->init("bigScreen", GL::UniformType::INT, 1);

	maximizeUnis = new GL::UniformTable(*maximize);
	maximizeUnis->init(
		"letterMap", GL::UniformType::INT, 1,
		"smallScreen", GL::UniformType::INT, 1,
		"numDivisions", GL::UniformType::IVEC2, 1,
		"freqBounds", GL::UniformType::VEC2, 1,
		"isColored", GL::UniformType::INT, 1,
		"correctiveExponent", GL::UniformType::FLOAT, 1
	);

	frequencies = new GL::UniformBufferTable(0);
	frequencies->init("freqs", GL::UniformType::FLOAT, 256);

}

const char* FontConverter::commonVertShader = " \
\
#version 430 core\n \
\
const vec2 coords[6] = vec2[]( \
	vec2(0.0, 0.0), \
	vec2(0.0, 1.0), \
	vec2(1.0, 1.0), \
	vec2(1.0, 1.0), \
	vec2(1.0, 0.0), \
	vec2(0.0, 0.0) \
	); \
\
out vec2 texCoords; \
\
void main() { \
	\
	gl_Position = vec4(coords[gl_VertexID] * 2.0 - 1.0, 0.0, 1.0); \
	texCoords = coords[gl_VertexID]; \
	\
} \
";

const char* FontConverter::minimizeFragShader = " \
\
#version 430 core\n \
 \
in vec2 texCoords; \
out vec4 fragColor; \
\
uniform sampler2D bigScreen; \
\
void main() { fragColor = vec4(texture(bigScreen, texCoords).rgb, 1.0); } \
";

const char* FontConverter::maximizeFragShader = " \
\
#version 430 core\n \
 \
in vec2 texCoords; \
out vec4 fragColor; \
\
uniform sampler2D letterMap; \
uniform sampler2D smallScreen; \
uniform ivec2 numDivisions; \
uniform vec2 freqBounds; \
uniform int isColored; \
uniform float correctiveExponent; \
\
layout(std140, binding = 0) uniform freqsBuffer { \
	float freqs[256]; \
}; \
\
vec2 scaleCoords(vec2 coords) { return fract(coords * vec2(numDivisions)); } \
\
float findClosestRank(float v) { \
	\
	int lh = 0; int rh = 255; int mid = 128; \
	\
	for (int i = 0; i < 10; i++) { \
		\
		if (v < freqs[mid]) { \
			\
			rh = mid; \
			mid = lh + (rh - lh) / 2; \
			\
		} \
		else { \
			\
			mid = mid + (rh - mid) / 2; \
			lh = mid; \
			\
		} \
		\
		if (lh == rh) break; \
		\
	} \
	\
	return float(lh); \
	\
} \
\
float sampleLetterByRank(float rank, vec2 coords) { \
	\
	coords.x = (coords.x + 255.0 - rank) / 256.0; \
	return texture(letterMap, coords).x; \
	\
} \
\
vec3 sampleLetter(vec2 coords) { \
	\
	vec3 samp = texture(smallScreen, coords).rgb; \
	samp = clamp(samp, 0.0, 1.0); \
	samp = pow(samp, vec3(correctiveExponent)); \
	\
	if (isColored == 0) samp.r = (samp.r + samp.g + samp.b) / 3.0; \
	\
	vec3 letterified = mix(vec3(freqBounds.x), vec3(freqBounds.y), samp); \
	vec2 scaledCoords = scaleCoords(coords); \
	\
	for (int i = 0; i < 3; i++) { \
		\
		letterified[i] = findClosestRank(letterified[i]); \
		letterified[i] = sampleLetterByRank(letterified[i], scaledCoords); \
		\
		if (isColored == 0) break; \
		\
	} \
	\
	letterified = freqBounds.x + letterified * (freqBounds.y - freqBounds.x); \
	letterified = pow(letterified, vec3(1.0 / correctiveExponent)); \
	return (isColored == 1) ? letterified : letterified.rrr; \
	\
} \
\
void main() { fragColor = vec4(sampleLetter(texCoords), 1.0); } \
";

#endif