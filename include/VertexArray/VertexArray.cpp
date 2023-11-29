
#include "./VertexArray.hpp"

GL::VertexAttribute::VertexAttribute(DataType dataType, unsigned int numComponents, bool convertToFloat, bool normalize)
	: dataType(dataType), convert(convertToFloat), norm(normalize) {

	numComponents = (numComponents >= 1u) ? numComponents : 1u;
	numComponents = (numComponents <= 4u) ? numComponents : 4u;
	this->numComponents = numComponents;

	static unsigned int dataSizes[] = { 2u, 4u, 1u, 2u, 4u, 1u, 2u, 4u };
	size = dataSizes[(int)dataType] * numComponents;

}

GL::DataType GL::VertexAttribute::getDataType() const { return dataType; }

unsigned int GL::VertexAttribute::getNumComponents() const { return numComponents; }

unsigned int GL::VertexAttribute::getSize() const { return size; }

bool GL::VertexAttribute::convertToFloat() const { return convert; }

bool GL::VertexAttribute::normalize() const { return norm; }