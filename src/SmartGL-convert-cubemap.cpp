
#include <iostream>
#include <string>

#include "Texture/TextureConverter.hpp"

int main(int argc, char** argv) {

#define arg(n) ((const char*) argv[n])
#define filepath(n) (std::string(std::string(arg(1)) + "/" + std::string(arg(n))).c_str())

    if (argc != 9 && argc != 15) {

        std::cout << "Invalid number of arguments. Usage: [image filepath] [output filename] [right] [left] [top] [bottom] [front] [back] [optional: ccRotation for each face]\n";
        return 1;

    }

    try {

        unsigned int rotations[6];

        if (argc == 15) {

            try { for (int i = 0; i < 6; i++) rotations[i] = std::stoul((const char*)argv[9 + i]); }
            catch (...) { throw GL::Exception("ccRotation arguments must be a positive integer."); }

        }
        else for (int i = 0; i < 6; i++) rotations[i] = 0;

        GL::Image faces[] = {
            GL::Image(filepath(3), GL::ColorFormat::RGB, rotations[0]),
            GL::Image(filepath(4), GL::ColorFormat::RGB, rotations[1]),
            GL::Image(filepath(5), GL::ColorFormat::RGB, rotations[2]),
            GL::Image(filepath(6), GL::ColorFormat::RGB, rotations[3]),
            GL::Image(filepath(7), GL::ColorFormat::RGB, rotations[4]),
            GL::Image(filepath(8), GL::ColorFormat::RGB, rotations[5])
        };

        GL::convertCubeMap(filepath(2), faces);

    }
    catch (GL::Exception e) { 
        
        std::cout << e.getMessage() << "\n";
        return 1;

    }

    return 0;

}