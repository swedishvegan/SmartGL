#ifndef UNIFORMTABLE_HPP
#define UNIFORMTABLE_HPP

#include <cstring>

#include "./../Program/Program.hpp"
#include "./ProgramUniforms.hpp"

namespace GL {

	class UniformTable : public ProgramUniforms {
	public:

		UniformTable();

		UniformTable(Program& program);

		template <typename T>
		T get(const char* name) const;

		template <typename T>
		void set(const char* name, T value);

		template <typename T>
		T getElement(const char* name, unsigned int index) const;

		template <typename T>
		void setElement(const char* name, unsigned int index, T value);

		void update();

		void update(Program& program);

	protected:

		bool isOwned;

		void update_commonCode(unsigned int progID);

		unsigned int get_commonCode(const char* name) const;

		void updateSE(unsigned int idx);

	};

}

#define _GL_UniformTable_getSet_commonCode(getOrSet) \
if (!isInitialized()) throw Exception("Attempt to " # getOrSet " the uniform \"" + std::string(name) + "\" from an uninitialized uniform table."); \
\
unsigned int idx = get_commonCode(name); \
\
if (idx == numUniforms) throw Exception("Invalid uniform name \"" + std::string(name) + "\" passed to uniform table's " # getOrSet " function."); \
if (!typeIsValid<T>(types[idx])) throw Exception("Invalid data type passed to uniform table's " # getOrSet " function (uniform name passed was \"" + std::string(name) + "\").");

template <typename T>
T GL::UniformTable::get(const char* name) const {

	_GL_UniformTable_getSet_commonCode(get);
	
	return *(T*)(data + offsets[idx]);

}

template <typename T>
void GL::UniformTable::set(const char* name, T value) {

	_GL_UniformTable_getSet_commonCode(set);

	updateSE(idx);
	*(T*)(data + offsets[idx]) = value;

}

template <typename T>
T GL::UniformTable::getElement(const char* name, unsigned int index) const {

	_GL_UniformTable_getSet_commonCode(get);
	if (index >= numElements[idx]) throw Exception("Invalid uniform array index " + std::to_string(index) + " passed to uniform buffer table's get function (uniform name passed was \"" + std::string(name) + "\").");
	
	return *(T*)(data + offsets[idx] + index * strides[idx]);

}

template <typename T>
void GL::UniformTable::setElement(const char* name, unsigned int index, T value) {

	_GL_UniformTable_getSet_commonCode(set);
	if (index >= numElements[idx]) throw Exception("Invalid uniform array index " + std::to_string(index) + " passed to uniform buffer table's set function (uniform name passed was \"" + std::string(name) + "\").");

	updateSE(idx);
	*(T*)(data + offsets[idx] + index * strides[idx]) = value;

}

#endif