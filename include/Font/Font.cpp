
#include "./Font.hpp"

GL::Program* GL::Font::fontRenderer = nullptr;
GL::UniformTable* GL::Font::fontRendererUnis = nullptr;

GL::Font::Font(FontLoader& loader, uint unit) { init(&loader, unit); }

float GL::Font::write(GL::Framebuffer& framebuffer, std::string message, GL::vec2 coords, GL::vec4 color, GL::FontWrap mode, float wrapDistance, bool NDC) const { return write(&framebuffer, message, coords, color, mode, wrapDistance, NDC); }

float GL::Font::write(std::string message, GL::vec2 coords, GL::vec4 color, GL::FontWrap mode, float wrapDistance, bool NDC) const { return write(nullptr, message, coords, color, mode, wrapDistance, NDC); }

GL::Font::~Font() {

	for (int i = 0; i < 256; i++) if (textures[i]) delete textures[i];
	if (loaderToDelete) delete loaderToDelete;

}

void GL::Font::init(GL::FontLoader* loader, uint unit) {

	if (!fontRenderer) {

		ShaderLoader fontLoader_vs(ShaderType::VERTEX);
		ShaderLoader fontLoader_fs(ShaderType::FRAGMENT);

		fontLoader_vs.init(Font::fontRenderer_vs, false);
		fontLoader_fs.init(Font::fontRenderer_fs, false);

		fontRenderer = new Program();
		fontRenderer->init(fontLoader_vs, fontLoader_fs);

		fontRendererUnis = new UniformTable(*fontRenderer);
		fontRendererUnis->init(
			"start", UniformType::VEC2, 1,
			"boxSize", UniformType::VEC2, 1,
			"screenSize", UniformType::VEC2, 1,
			"tex", UniformType::SAMPLER, 1,
			"textColor", UniformType::VEC4, 1
		);
		fontRendererUnis->set<int>("tex", unit);

	}

	this->unit = unit;
	this->loader = loader;

	for (uint i = 0u; i < 256u; i++) {

		uchar c = (uchar)i;
		auto data = loader->getGlyphInfo(c);

		if (data.size.x * data.size.y > 0) {

			textures[i] = new CoupledTexture2D<float>(data.size.x, data.size.y, unit, GL::ColorFormat::R, DataType::F16);
			for (uint x = 0u; x < data.size.x; x++) for (uint y = 0u; y < data.size.y; y++) textures[i]->setPixel(x, y, 0u, (float)loader->getGlyphEntry(c, ivec2(x, y)) / 255.0f);
			textures[i]->writeToGPU();

		}
		else textures[i] = nullptr;

	}

}

float GL::Font::write(GL::Framebuffer* framebuffer, std::string message, GL::vec2 coords, GL::vec4 color, GL::FontWrap mode, float wrapDistance, bool NDC) const {

	bool shouldDraw = color.w > 0.001f;

	vec2 ss((framebuffer) ? vec2(framebuffer->getWidth(), framebuffer->getHeight()) : vec2(getScreenSize()));
	float startX = coords.x;

	if (NDC) {

		coords = ss * (coords + 1.0f) / 2.0f;
		wrapDistance = ss.x * wrapDistance / 2.0f;

	}

	if (shouldDraw) {

		fontRendererUnis->set("screenSize", ss);
		fontRendererUnis->set("textColor", color);

		fontRenderer->use();
		if (framebuffer) framebuffer->use(); else Framebuffer::useDefault();
		Render::drawDefault();

	}

	bool done = false;
	int cur_i = 0;

	float maxWidth = 0.0f;

	while (!done) {

		int next_i = (mode == FontWrap::WORD) ? getNextLine(cur_i, message, startX, wrapDistance) : message.size();
		int i;

		if (mode != FontWrap::WORD || next_i == message.size()) done = true;

		for (i = cur_i; i < next_i; i++) {

			uchar c = message[i];
			int idx = (int)c % 256;

			auto data = loader->getGlyphInfo(c);

#define _GL_Font_write_moveCoordsDown() maxWidth = GL::max(maxWidth, coords.x - startX); coords = vec2(startX, coords.y - loader->getBounds().height);

			if (mode == FontWrap::LETTER and coords.x + data.advance > wrapDistance) { _GL_Font_write_moveCoordsDown() }

			if (shouldDraw && textures[idx]) {

				auto& tex = *textures[idx];

				fontRendererUnis->set("start", coords + vec2(data.offset));
				fontRendererUnis->set("boxSize", vec2(data.size));
				fontRendererUnis->update();
				tex.bind();

				Render::clearBuffers(GL::DEPTH_BUFFER);
				Render::drawDefault();

			}

			coords.x += data.advance;

		}

		cur_i = i;
		_GL_Font_write_moveCoordsDown()

	}

	return maxWidth;

}

int GL::Font::getNextLine(int cur, std::string s, float start, float limit) const {

	int next = -1;

	for (int i = cur; i < s.size(); i++) {

		uchar c = s[i];
		auto data = loader->getGlyphInfo(c);

		if (c == ' ' and start <= limit) next = i;
		if (start > limit) return max(cur + 1, (next < 0) ? i - 1 : next + 1);

		start += data.advance;

	}

	return s.size();

}

const char* GL::Font::fontRenderer_vs = " \
\
#version 430 core\n \
\
uniform vec2 start; \
uniform vec2 boxSize; \
uniform vec2 screenSize; \
\
out vec2 texCoords; \
\
const vec2 vertices[] = vec2[]( \
	vec2(0.0, 0.0), \
	vec2(1.0, 0.0), \
	vec2(1.0, 1.0), \
	vec2(1.0, 1.0), \
	vec2(0.0, 1.0), \
	vec2(0.0, 0.0) \
); \
\
vec2 toScreenSpace(vec2 pixels) { return (pixels / screenSize) * 2.0 - 1.0; } \
\
void main() { \
	\
	vec2 vertex = vertices[gl_VertexID]; \
	vec2 s = toScreenSpace(start); \
	vec2 e = toScreenSpace(start + boxSize); \
	\
	texCoords = vertex; \
	gl_Position = vec4(mix(s, e, vertex), -1.0, 1.0); \
	\
}";

const char* GL::Font::fontRenderer_fs = " \
\
#version 430 core\n \
\
in vec2 texCoords; \
\
out vec4 fragColor; \
\
uniform sampler2D tex; \
uniform vec4 textColor; \
\
void main() { \
	\
	fragColor = textColor; \
	fragColor.a *= texture(tex, texCoords).r; \
	\
}";
