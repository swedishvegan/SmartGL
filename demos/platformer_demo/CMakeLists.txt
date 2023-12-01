add_executable(platformer_demo platformer_demo.cpp)

target_link_libraries(platformer_demo SmartGL glfw)
target_compile_definitions(platformer_demo PRIVATE CUR_DIRECTORY="${CMAKE_SOURCE_DIR}/demos/platformer_demo")