
#include "./TextureConverter.hpp"

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