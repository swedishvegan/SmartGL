cmake_minimum_required(VERSION 3.7...3.27)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
	cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	set(CMAKE_CXX_FLAGS "/EHsc")
endif()

#set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_STANDARD_REQUIRED True)

project(SmartGL)

file(GLOB_RECURSE SmartGL_SOURCES include/*.hpp include/*.cpp)

find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(assimp REQUIRED)
find_package(Freetype REQUIRED)
find_package(Bullet REQUIRED)

set(SmartGL_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/include;${GLEW_INCLUDE_DIRS};${BULLET_INCLUDE_DIRS};${FREETYPE_INCLUDE_DIRS}")

include_directories("${SmartGL_INCLUDE_DIRS};${GLFW_INCLUDE_DIRS};${ASSIMP_INCLUDE_DIRS};")

subdirs(bin)
subdirs(demos)

set(CMAKE_EXPORT_PACKAGE_REGISTRY TRUE)
export(PACKAGE SmartGL)