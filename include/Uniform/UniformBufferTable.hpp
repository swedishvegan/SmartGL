#ifndef UNIFORMBUFFERTABLE_HPP
#define UNIFORMBUFFERTABLE_HPP

#include "./BufferTable.hpp"

namespace GL {

	class UniformBufferTable : public BufferTable {
	public:
		UniformBufferTable(unsigned int bindingPoint);
	};

}

#endif