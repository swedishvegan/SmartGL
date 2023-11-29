#ifndef TEXTURECONVERTER_HPP
#define TEXTURECONVERTER_HPP

#include <fstream>

#include "./Image.hpp"
#include "./../util/Exception.hpp"
#include "./../util/BinaryFile.hpp"

namespace GL {

	void convertCubeMap(const char* filePath, Image faces[6]);

	template <int _>
	struct _dummy {

		static char _cubeMap_verif[7];

	};

}

template <int _>
char GL::_dummy<_>::_cubeMap_verif[7];

#define GL_loadCubeMapFromFile(cubeMapName, filePath, unit, type) \
std::ifstream cubeMapName ## _loader(filePath, std::ios::binary); \
if (!cubeMapName ## _loader.is_open()) throw GL::Exception("Failed to open cube map file."); \
\
cubeMapName ## _loader.read((char*)GL::_dummy<0>::_cubeMap_verif, 7); \
if (GL::_dummy<0>::_cubeMap_verif[0] != 'c' || GL::_dummy<0>::_cubeMap_verif[1] != 'u' || GL::_dummy<0>::_cubeMap_verif[2] != 'b' || GL::_dummy<0>::_cubeMap_verif[3] != 'e' || GL::_dummy<0>::_cubeMap_verif[4] != 'm' || GL::_dummy<0>::_cubeMap_verif[5] != 'a' || GL::_dummy<0>::_cubeMap_verif[6] != 'p') throw GL::Exception("Cube map file verification failed."); \
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