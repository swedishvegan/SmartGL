#ifndef BINARY_FILE_HPP
#define BINARY_FILE_HPP

#include <fstream>
#include <stdint.h>
#include "util/Exception.hpp"

class ReadBinaryFile {
public:
	
	bool isEOF();
	
	int getFileLength();

	ReadBinaryFile(const char* filepath, int buffer_size = 1024);

	bool readBit();

	char readByte();

	template <typename T>
	T read();

	~ReadBinaryFile();

private:

	std::ifstream file;
	int fileLength;
	char* buffer;
	int bufferSize;
	int pos;
	int bytePos;
	int bitPos;
	bool isLittleEndian;

};

class WriteBinaryFile {
public:

	int getFileLength();

	WriteBinaryFile(const char* filepath, int buffer_size = 1024);

	void writeBit(bool bit);

	void writeByte(char byte);

	template <typename T>
	void write(T val);

	void writeRawData(char* data, int numElements);

	void forceBufferWrite();

	~WriteBinaryFile();

private:

	std::ofstream file;
	char* buffer;
	int bufferSize;
	int fileSize;
	int bytePos;
	int bitPos;
	bool isLittleEndian;

};

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

template <typename T>
T ReadBinaryFile::read() {

	if (sizeof(T) == 1)
		return (T)readByte();

	else {

		if (bitPos > 0) {

			bitPos = 0;
			pos++;
			bytePos++;

		}
		if (bytePos == bufferSize) {

			bytePos = 0;
			file.read(buffer, bufferSize);

		}

#define _ReadBinaryFile_read(size) { \
\
	union { \
		T a; \
		char b[size]; \
	} src, dst; \
	\
	for (int i = 0; i < size; i++) { \
		\
		src.b[i] = buffer[bytePos]; \
		pos++; \
		bytePos++; \
		\
		if (bytePos == bufferSize) { \
			\
			bytePos = 0; \
			file.read(buffer, bufferSize); \
			\
		} \
	} \
	\
	if (isLittleEndian) return src.a; \
	\
	for (int i = 0; i < size; i++) dst.b[i] = src.b[size - 1 - i]; \
	return dst.a; \
	\
}

#define _RBF_case(size) if (sizeof(T) == size) _ReadBinaryFile_read(size)

		_RBF_case(2) _RBF_case(4) _RBF_case(8) _RBF_case(16) _RBF_case(32) _RBF_case(64);

		throw GL::Exception("A binary file reader can only read data types with size 2^i with i between 0 and 6.");

	}
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

template <typename T>
void WriteBinaryFile::write(T val) {

	if (sizeof(T) == 1)
		writeByte((char)val);

	else {

		if (bitPos > 0) {

			bitPos = 0;
			fileSize++;
			bytePos++;

		}
		if (bytePos == bufferSize) {

			bytePos = 0;
			file.write(buffer, bufferSize);

		}

#define _WriteBinaryFile_write(size) { \
\
	union { \
		T a; \
		char b[size]; \
	} src, dst; \
	src.a = val; \
	\
	for (int i = 0; i < size; i++) { \
		\
		dst.b[i] = (isLittleEndian) ? src.b[i] : src.b[size - 1 - i]; \
		buffer[bytePos] = dst.b[i]; \
		fileSize++; \
		bytePos++; \
		\
		if (bytePos == bufferSize) { \
			\
			bytePos = 0; \
			file.write(buffer, bufferSize); \
			\
		} \
	} \
	\
	success = true; \
	\
}

#define _WBF_case(size) if (sizeof(T) == size) _WriteBinaryFile_write(size)

		bool success = false;
		_WBF_case(2) _WBF_case(4) _WBF_case(8) _WBF_case(16) _WBF_case(32) _WBF_case(64);

		if (!success) throw GL::Exception("A binary file writer can only write data types with size 2^i with i between 0 and 6.");

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

#endif