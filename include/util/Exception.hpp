#ifndef EXCEPTION_HPP
#define EXCEPTION_HPP

#include <string>

namespace GL {

	class Exception {
	public:

		Exception(std::string message);

		std::string getMessage() const;

	private:

		std::string message;

	};

}

#endif