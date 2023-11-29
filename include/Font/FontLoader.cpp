
#include "./FontLoader.hpp"

#ifndef NO_FREETYPE
FT_Library GL::FontLoader::library;
FT_Error GL::FontLoader::error;
bool GL::FontLoader::needsInit = true;

GL::FontLoader::FontLoader(const char* filepath, int size_pts, int ppi) {

	if (needsInit) {

		error = FT_Init_FreeType(&library);
		needsInit = false;

	}
	
	if (error) throw Exception("Error initializing FT.");

	FT_Face typeface;
	FT_Error e = FT_New_Face(library, filepath, 0, &typeface);
	if (e) throw Exception("Error creating FT face.");

	e = FT_Set_Char_Size(typeface, 0, size_pts * 64, ppi, ppi);
	if (e) throw Exception("Error setting FT char size.");

	bounds.ascender = typeface->size->metrics.ascender;
	bounds.descender = typeface->size->metrics.descender;
	bounds.height = typeface->size->metrics.height;
	bounds.maxAdvance = typeface->size->metrics.max_advance;
	bounds.ascender /= 64; bounds.descender /= 64; bounds.height /= 64; bounds.maxAdvance /= 64;
	
	for (int i = 0; i < 256; i++) {

		FT_UInt glyphIndex = FT_Get_Char_Index(typeface, (FT_ULong)i);
		FT_Load_Glyph(typeface, glyphIndex, FT_LOAD_DEFAULT);
		if (typeface->glyph->format != FT_GLYPH_FORMAT_BITMAP) FT_Render_Glyph(typeface->glyph, FT_RENDER_MODE_NORMAL);
		
		info[i] = GlyphInfo{

			ivec2(typeface->glyph->bitmap.width, typeface->glyph->bitmap.rows),
			ivec2(typeface->glyph->bitmap_left, (typeface->glyph->bitmap_top - (int)typeface->glyph->bitmap.rows)),
			(float)typeface->glyph->advance.x / 64.0f,
			nullptr

		};

		int s = info[i].size.x * info[i].size.y;
		info[i].data = (s > 0) ? new uchar[s] : nullptr;
		for (int j = 0; j < s; j++) info[i].data[j] = typeface->glyph->bitmap.buffer[j];

	}

	FT_Done_Face(typeface);
	
}
#endif

void GL::FontLoader::save(const char* filepath) const {

	WriteBinaryFile wbf(filepath);

	wbf.write('F');
	wbf.write('O');
	wbf.write('N');
	wbf.write('T');

	wbf.write(bounds.ascender);
	wbf.write(bounds.descender);
	wbf.write(bounds.height);
	wbf.write(bounds.maxAdvance);

	for (int i = 0; i < 256; i++) {

		GlyphInfo gInfo = info[i];

		for (int i = 0; i < 2; i++) wbf.write(gInfo.size[i]);
		for (int i = 0; i < 2; i++) wbf.write(gInfo.offset[i]);
		wbf.write(gInfo.advance);
		
		if (gInfo.data) for (int i = 0; i < gInfo.size.x * gInfo.size.y; i++) wbf.write(gInfo.data[i]);

	}

}

#ifdef NO_FREETYPE
GL::FontLoader::FontLoader(const char* filepath) {

	ReadBinaryFile rbf(filepath);

	char verify[4];
	for (int i = 0; i < 4; i++) verify[i] = rbf.read<char>();

	if (!(
		verify[0] == 'F' &&
		verify[1] == 'O' &&
		verify[2] == 'N' &&
		verify[3] == 'T'
	)) throw GL::Exception("Invalid file type passed to FontLoader constructor.");

	bounds.ascender = rbf.read<int>();
	bounds.descender = rbf.read<int>();
	bounds.height = rbf.read<int>();
	bounds.maxAdvance = rbf.read<int>();

	for (int i = 0; i < 256; i++) {

		GlyphInfo gInfo{ };

		for (int i = 0; i < 2; i++) gInfo.size[i] = rbf.read<int>();
		for (int i = 0; i < 2; i++) gInfo.offset[i] = rbf.read<int>();
		gInfo.advance = rbf.read<float>();

		int s = gInfo.size.x * gInfo.size.y;
		gInfo.data = (s > 0) ? new uchar[s] : nullptr;
		for (int i = 0; i < s; i++) gInfo.data[i] = rbf.read<uchar>();

		info[i] = gInfo;

	}

}
#endif

GL::FontLoader::Bounds GL::FontLoader::getBounds() const { return bounds; }

GL::FontLoader::GlyphInfo GL::FontLoader::getGlyphInfo(uchar c) const { return info[(uint)c]; }

uchar GL::FontLoader::getGlyphEntry(uchar c, GL::ivec2 coords) const {

	coords = GL::clamp(coords, GL::ivec2(), getGlyphInfo(c).size - 1);
	int idx = (getGlyphInfo(c).size.y - coords.y - 1) * getGlyphInfo(c).size.x + coords.x;
	uchar* glyphData = getGlyphData(c);

	return (glyphData) ? glyphData[idx] : 0;

}

GL::FontLoader::~FontLoader() { for (int i = 0; i < 256; i++) delete[] info[i].data; }

uchar* GL::FontLoader::getGlyphData(uchar c) const { return info[(uint)c].data; }