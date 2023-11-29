
#include "./../util/Exception.hpp"

GL::Exception::Exception(std::string message) : message(message) { };

std::string GL::Exception::getMessage() const { return message; }