add_executable(physics_demo physics_demo.cpp)

target_link_libraries(physics_demo SmartGL glfw)
target_compile_definitions(physics_demo PRIVATE CUR_DIRECTORY="${CMAKE_SOURCE_DIR}/demos/physics_demo")