add_executable(ascii_demo ascii_demo.cpp)

target_link_libraries(ascii_demo SmartGL glfw)
target_compile_definitions(ascii_demo PRIVATE FREETYPE)