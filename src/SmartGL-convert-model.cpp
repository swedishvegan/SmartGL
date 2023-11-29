
#include <iostream>

#include "Model/ModelConverter.hpp"

int main(int argc, char** argv) {

    if (argc != 3) {

        std::cout << "Invalid number of arguments. Usage: [input filename] [output filename]\n";
        return 1;

    }

    try { GL::ModelConverter converter(argv[1], argv[2]); }
    catch (GL::Exception e) { 
        
        std::cout << e.getMessage() << "\n";
        return 1;

    }

    return 0;

}