#ifndef FONT_HPP
#define FONT_HPP

#include <string>

#include "./../util/util.hpp"
#include "./../Framebuffer/Framebuffer.hpp"
#include "./../Texture/Texture2D.hpp"
#include "./../Program/Program.hpp"
#include "./../Uniform/UniformTable.hpp"
#include "./../VertexArray/VertexArray.hpp"
#include "./FontLoader.hpp"

namespace GL {

	class Font : public _util {
	public:

		Font(FontLoader& loader, uint unit = 0u);

		float write(Framebuffer& framebuffer, std::string message, vec2 coords, vec4 color = vec4(1.0f), FontWrap mode = FontWrap::NONE, float wrapDistance = 0.0f, bool NDC = false) const;

		float write(std::string message, vec2 coords, vec4 color = vec4(1.0f), FontWrap mode = FontWrap::NONE, float wrapDistance = 0.0f, bool NDC = false) const;

		~Font();

	protected:

		CoupledTexture2D<float>* textures[256];
		uint unit;
		FontLoader* loader;
		FontLoader* loaderToDelete = nullptr;

		static Program* fontRenderer;
		static UniformTable* fontRendererUnis;

		static const char* fontRenderer_vs;
		static const char* fontRenderer_fs;

		void init(FontLoader*, uint);

		float write(Framebuffer*, std::string, vec2, vec4, FontWrap, float, bool) const;

		int getNextLine(int, std::string, float, float) const;

	};

}

#endif