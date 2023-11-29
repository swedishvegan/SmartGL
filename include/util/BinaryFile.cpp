
#include "./BinaryFile.hpp"

bool ReadBinaryFile::isEOF() { return (pos >= fileLength); }

int ReadBinaryFile::getFileLength() { return fileLength; }

ReadBinaryFile::ReadBinaryFile(const char* filepath, int buffer_size) : file(filepath, std::ios::binary) {

	if (!file.is_open()) throw GL::Exception("Failed to read binary file from filepath \"" + std::string(filepath) + "\".");

	file.seekg(0, file.end);
	fileLength = file.tellg();

	file.seekg(0, file.beg);
	fileLength -= file.tellg();

	bufferSize = buffer_size;
	buffer = new char[bufferSize];
	pos = 0;
	bytePos = 0;
	bitPos = 0;

	file.read(buffer, bufferSize);

	uint16_t x = 1u;
	isLittleEndian = *((uint8_t*)&x) == 1u;

}

bool ReadBinaryFile::readBit() {

	bool bit = (bool)((((uint8_t)buffer[bytePos]) >> bitPos) % 2);
	bitPos++;

	if (bitPos == 8) {

		bitPos = 0;
		pos++;
		bytePos++;

	}
	if (bytePos == bufferSize) {

		bytePos = 0;
		file.read(buffer, bufferSize);

	}

	return bit;

}

char ReadBinaryFile::readByte() {

	if (bitPos > 0) {

		bitPos = 0;
		pos++;
		bytePos++;

	}
	if (bytePos == bufferSize) {

		bytePos = 0;
		file.read(buffer, bufferSize);

	}

	char byte = buffer[bytePos];
	pos++;
	bytePos++;

	if (bytePos == bufferSize) {

		bytePos = 0;
		file.read(buffer, bufferSize);

	}

	return byte;

}

ReadBinaryFile::~ReadBinaryFile() {

	file.close();
	delete[] buffer;

}

int WriteBinaryFile::getFileLength() { return fileSize; }

WriteBinaryFile::WriteBinaryFile(const char* filepath, int buffer_size) : file(filepath, std::ios::binary) {

	if (!file.is_open()) throw GL::Exception("Failed to write binary file to filepath \"" + std::string(filepath) + "\".");

	file.seekp(0, file.beg);

	bufferSize = buffer_size;
	buffer = new char[bufferSize];

	fileSize = 0;
	bytePos = 0;
	bitPos = 0;

	uint16_t x = 1u;
	isLittleEndian = *((uint8_t*)&x) == 1u;

}

void WriteBinaryFile::writeBit(bool bit) {

	uint8_t byte = (uint8_t)buffer[bytePos];
	byte = (byte & ~((uint8_t)(255u << bitPos))) | (((uint8_t)bit) << bitPos);
	buffer[bytePos] = (char)byte;

	bitPos++;
	if (bitPos == 8) {

		bitPos = 0;
		fileSize++;
		bytePos++;

	}
	if (bytePos == bufferSize) {

		bytePos = 0;
		file.write(buffer, bufferSize);

	}

}

void WriteBinaryFile::writeByte(char byte) {

	if (bitPos > 0) {

		bitPos = 0;
		fileSize++;
		bytePos++;

	}
	if (bytePos == bufferSize) {

		bytePos = 0;
		file.write(buffer, bufferSize);

	}

	buffer[bytePos] = byte;
	fileSize++;
	bytePos++;

	if (bytePos == bufferSize) {

		bytePos = 0;
		file.write(buffer, bufferSize);

	}

}

void WriteBinaryFile::writeRawData(char* data, int numElements) {

	forceBufferWrite();
	file.write(data, numElements);
	fileSize += numElements;

}

void WriteBinaryFile::forceBufferWrite() {

	if (bitPos > 0) {

		bitPos = 0;
		bytePos++;

	}
	if (bytePos > 0) {

		file.write(buffer, bytePos);
		bytePos = 0;

	}

}

WriteBinaryFile::~WriteBinaryFile() {

	if (bytePos > 0) file.write(buffer, bytePos);
	file.close();
	delete[] buffer;

}
