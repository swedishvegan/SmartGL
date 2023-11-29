
#include "./ModelInstanceBuffer.hpp"

GL::ModelInstanceBuffer::ModelInstanceBuffer(unsigned int numInstances) : modelTable(0u), normalTable(1u) {

	len = numInstances;
	if (len == 0u) len++;

	modelTable.init("modelMatrices", UniformType::MAT4, len);
	normalTable.init("normalMatrices", UniformType::MAT3, len);

}

void GL::ModelInstanceBuffer::setModelMatrix(unsigned int index, GL::mat4 model) {

	mat3 normalMatrix(model);
	normalMatrix = transpose(inverse(normalMatrix));

	modelTable.setElement<mat4>("modelMatrices", index % len, model);
	normalTable.setElement<mat3>("normalMatrices", index % len, normalMatrix);

}

GL::mat4 GL::ModelInstanceBuffer::getModelMatrix(unsigned int index) const { return modelTable.getElement<mat4>("modelMatrices", index % len); }

GL::mat3 GL::ModelInstanceBuffer::getNormalMatrix(unsigned int index) const { return normalTable.getElement<mat3>("normalMatrices", index % len); }

unsigned int GL::ModelInstanceBuffer::getLength() const { return len; }

void GL::ModelInstanceBuffer::bind() const {

	modelTable.bind();
	normalTable.bind();

}

void GL::ModelInstanceBuffer::update() {
	
	modelTable.update();
	normalTable.update();

}