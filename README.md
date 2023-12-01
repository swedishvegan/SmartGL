# SmartGL

OpenGL made easy.

1. [About this library](#pt1)
2. [Getting started](#pt2)
    1. [Building on Windows](#pt2.1)
    2. [Building on Linux](#pt2.2)
    3. [Running the demos](#pt2.3)
    4. [Using the SmartGL library](#pt2.4)

# About this library <a name="pt1"></a>

The SmartGL library is at its core two things:
1. A low-level interface designed to streamline common OpenGL workflows and make using OpenGL easier and less of a hassle
2. A high-level interface for loading and rendering models in a 3D scene, with support for physically-based rendering, shadow mapping, physics integration, custom model shaders, and some other handy features

There is currently no documentation for the SmartGL library. The best way to learn how to use it is simply by looking at the source code for the demos I've provided. (See [Running the demos](#pt2.3) and the relevant source code in the ```demos``` folder.) If you're already familiar with OpenGL, most of the classes and wrappers will be fairly intuitive to use.

I wrote this library for fun during the pandemic back in 2021. I did it to challenge myself and learn more about graphics. Although my programming skills have come a long way since then, I'm happy with what I achieved and I think this library still holds up fairly well.

### Disclaimer

I am no longer actively developing or maintaining the SmartGL library. I've moved on to bigger and better projects, and plan to create a more extensive Vulkan-based graphics library next year designed to be used with the [Lax programming language](https://github.com/swedishvegan/complax).

There are a few bugs at the moment related to loading certain model formats, especially with animated models. This might be a platform-specific or driver-specific issue, as I never had issues loading animated models on my old Windows PC with an Nvidia card. For the most part, it works correctly, but this is something to beware of, especially when downloading sketchy models off the internet like I do.

Use the SmartGL library at your own risk :)

# Getting started <a name="pt2"></a>

SmartGL has a lot of dependencies, as is true for most graphics applications. For this reason, I've put considerable work into making it easy to manage these dependencies across multiple platforms.

To build SmartGL, you will need:
- a C++ compiler (MSVC on Windows and G++ on Linux)
- Git
- CMake
- a package manager to install dependencies (vcpkg on Windows, aptitude on Linux)

__If you don't have access to a package manager, you'll need to manually install all of the dependencies and tell CMake where to find them.__

Go ahead and clone the repository to your machine, and then follow these platform-specific directions:

## Building on Windows <a name="pt2.1"></a>

The easiest way to build SmartGL on Windows is to use the ```vcpkg``` package manager. If you don't already have it, head over to their website and follow their build/installation instructions. It was super easy and took me about 5 minutes.

Once you have ```vcpkg``` installed, run the ```install_dependencies.bat``` file in the SmartGL folder. I recommend running it from the command line so that you'll be able to see if there are any error messages.

Once dependencies are installed, go ahead and generate the build files with CMake: &nbsp; ```cmake . -DCMAKE_TOOLCHAIN_FILE="[path to vcpkg.exe on your machine]/scripts/buildsystems/vcpkg.cmake"```

If all went well, CMake should have generated a ```SmartGL.sln``` build file. Go ahead and open this file in Visual Studio and build the project in Release mode. (I don't think the linker is configured to work with certain libraries in debug mode right now, but you can try.)

## Building on Linux <a name="pt2.2"></a>

If you're using Ubuntu/Debian, you can just use ```apt-get``` to install all the dependencies. Otherwise, you might need to use a different package manager or manually install the dependencies. If you have ```apt-get``` then go ahead and run the shell script: &nbsp; ```install_dependencies.sh```

You might need to edit permissions on the file first: &nbsp; ```chmod a+rwx install_dependencies.sh```

It will probably also ask you for a password since you need sudo permissions to install new packages.

Now, go ahead and generate the build files with CMake: &nbsp; ```cmake .```

If all went well, CMake should have generated a ```Makefile``` for SmartGL. Go ahead and run ```Make``` to build the project.

## Running the demos <a name="pt2.3"></a>

Go ahead and try out the demos once the library is successfully built. First, try out ```demos/basic_demo```. When you run the executable, you should see something like this:

![basic_demo](/readme_assets/basic_demo.png)

The physics demo showcases Bullet physics integration.

The platformer demo is a simple physics-based platformer framework that showcases the use of custom shaders and procedural generation in the PBR pipline.

The ASCII demo showcases loading text, and also features a fun little real-time ASCII renderer that can be used to draw a 3D scene entirely in ASCII.

## Using the SmartGL library <a name="pt2.4"></a>

Linking your other C++ projects up with SmartGL is pretty easy if you're using CMake:

```
find_package(SmartGL REQUIRED)
target_link_libraries([your target] SmartGL)
```

You might need to also include a window creation library. In the demos, I use GLFW for this. Feel free to look at how I did this in CMake, and copy it into your own build files.