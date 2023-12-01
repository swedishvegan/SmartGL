add_executable(basic_demo basic_demo.cpp)

target_link_libraries(basic_demo SmartGL glfw)
target_compile_definitions(basic_demo PRIVATE CUR_DIRECTORY="${CMAKE_SOURCE_DIR}/demos/basic_demo")