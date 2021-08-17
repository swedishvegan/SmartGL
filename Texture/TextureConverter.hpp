#ifndef TEXTURECONVERTER_HPP
#define TEXTURECONVERTER_HPP

#include <fstream>

#include "Texture/Image.hpp"
#include "util/Exception.hpp"
#include "util/BinaryFile.hpp"

namespace GL {

	void convertCubeMap(const char* filePath, Image faces[6]);

	static char _cubeMap_verify[7];

}

void GL::convertCubeMap(const char* filePath, Image faces[6]) {

	for (int i = 0; i < 6; i++) if (faces[i].getError()) throw Exception(faces[i].getError());

	unsigned int w = faces[0].getWidth();
	ColorFormat format = faces[0].getFormat();
	if (w != faces[0].getHeight()) throw Exception("A cube map face's width must equal its height, but the dimensions of the given cube map are " + std::to_string(w) + "x" + std::to_string(faces[0].getHeight()) + ".");
	for (int i = 1; i < 6; i++) if (faces[i].getWidth() != w || faces[i].getHeight() != w) throw Exception("All cube map faces must be the same size, but face " + std::to_string(i) + " has dimensions " + std::to_string(faces[i].getWidth()) + "x" + std::to_string(faces[i].getHeight()) + " versus face 0, which has dimensions " + std::to_string(w) + "x" + std::to_string(w) + ".");
	for (int i = 1; i < 6; i++) if (faces[i].getFormat() != format) throw Exception("All cube map faces must have the same format, but face " + std::to_string(i) + " has format " + std::to_string((int)faces[i].getFormat()) + " versus face 0, which has format " + std::to_string((int)format) + ".");

	WriteBinaryFile wbf(filePath);

	wbf.writeRawData((char*)"cubemap", 7);
	wbf.write<unsigned int>(w);
	wbf.write<unsigned int>((unsigned int)format);
	for (int i = 0; i < 6; i++) wbf.writeRawData((char*)faces[i].getData(), w * w * ((unsigned int)format + 1u));

}

#define GL_loadCubeMapFromFile(cubeMapName, filePath, unit, type) \
std::ifstream cubeMapName ## _loader(filePath, std::ios::binary); \
if (!cubeMapName ## _loader.is_open()) throw GL::Exception("Failed to open cube map file."); \
\
cubeMapName ## _loader.read((char*)GL::_cubeMap_verify, 7); \
if (GL::_cubeMap_verify[0] != 'c' || GL::_cubeMap_verify[1] != 'u' || GL::_cubeMap_verify[2] != 'b' || GL::_cubeMap_verify[3] != 'e' || GL::_cubeMap_verify[4] != 'm' || GL::_cubeMap_verify[5] != 'a' || GL::_cubeMap_verify[6] != 'p') throw GL::Exception("Cube map file verification failed."); \
\
unsigned int cubeMapName ## _w, cubeMapName ## _format; \
cubeMapName ## _loader.read((char*)&cubeMapName ## _w, 4); \
cubeMapName ## _loader.read((char*)&cubeMapName ## _format, 4); \
\
unsigned char* cubeMapName ## _data = new unsigned char[cubeMapName ## _w * cubeMapName ## _w * (cubeMapName ## _format + 1u)]; \
GL::ImageTextureCubeMap cubeMapName(cubeMapName ## _w, unit, (GL::ColorFormat)cubeMapName ## _format, type); \
for (int cubeMapName ## _i = 0; cubeMapName ## _i < 6; cubeMapName ## _i++) { \
	\
	cubeMapName ## _loader.read((char*)cubeMapName ## _data, cubeMapName ## _w * cubeMapName ## _w * (cubeMapName ## _format + 1u)); \
	cubeMapName.setFaceData((CubeMapFace)cubeMapName ## _i, cubeMapName ## _data); \
	\
} \
delete[] cubeMapName ## _data;

#endif