#ifndef MODELINSTANCEBUFFER_HPP
#define MODELINSTANCEBUFFER_HPP

#include "./../Uniform/ShaderStorageBufferTable.hpp"

namespace GL {

	class ModelInstanceBuffer : public _util {
	public:

		ModelInstanceBuffer(unsigned int numInstances);

		void setModelMatrix(unsigned int index, mat4 model);

		mat4 getModelMatrix(unsigned int index) const;

		mat3 getNormalMatrix(unsigned int index) const;

		unsigned int getLength() const;

		void bind() const;

		void update();

	protected:

		unsigned int len;
		ShaderStorageBufferTable modelTable;
		ShaderStorageBufferTable normalTable;

	};

}

#endif