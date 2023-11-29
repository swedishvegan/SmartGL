#ifndef FONTLOADER_HPP
#define FONTLOADER_HPP

#include "./../util/GL-math.hpp"
#include "./../util/BinaryFile.hpp"

#ifndef NO_FREETYPE
#include "ft2build.h"
#include FT_FREETYPE_H
#endif

typedef unsigned char uchar;
typedef unsigned int uint;

namespace GL {

	class FontLoader {
	public:

		struct GlyphInfo {

			GL::ivec2 size;
			GL::ivec2 offset;
			float advance;

			uchar* data;

		};

		struct Bounds { int ascender, descender, height, maxAdvance; };

#ifndef NO_FREETYPE
		FontLoader(const char* filepath, int size_pts, int ppi);
#else
		FontLoader(const char* filepath);
#endif

		void save(const char* filepath) const;

		Bounds getBounds() const;

		GlyphInfo getGlyphInfo(uchar c) const;

		uchar getGlyphEntry(uchar c, GL::ivec2 coords) const;

		~FontLoader();

	private:

		GlyphInfo info[256];
		Bounds bounds;

#ifndef NO_FREETYPE
		static FT_Library library;
		static FT_Error error;
		static bool needsInit;
#endif

		uchar* getGlyphData(uchar c) const;

	};

}

#endif