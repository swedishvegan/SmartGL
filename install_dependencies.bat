vcpkg install glew --triplet x64-windows
vcpkg install glfw3 --triplet x64-windows
vcpkg install assimp --triplet x64-windows
vcpkg install freetype --triplet x64-windows
vcpkg install bullet3 --triplet x64-windows
cmake . -DCMAKE_TOOLCHAIN_FILE="C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build