#ifndef BINARY_FILE_HPP
#define BINARY_FILE_HPP

#include <fstream>
#include <stdint.h>
#include "./Exception.hpp"

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

#endif