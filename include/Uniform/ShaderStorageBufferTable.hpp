#ifndef SHADERSTORAGEBUFFERTABLE_HPP
#define SHADERSTORAGEBUFFERTABLE_HPP

#include "./BufferTable.hpp"

namespace GL {

	class ShaderStorageBufferTable : public BufferTable {
	public:
		ShaderStorageBufferTable(unsigned int bindingPoint);
	};

}

#endif