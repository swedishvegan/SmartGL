
add_library(SmartGL STATIC ${SmartGL_SOURCES})
add_executable(SmartGL-convert-model ${CMAKE_SOURCE_DIR}/src/SmartGL-convert-model.cpp)
add_executable(SmartGL-convert-cubemap ${CMAKE_SOURCE_DIR}/src/SmartGL-convert-cubemap.cpp)

target_sources(SmartGL PRIVATE ${SmartGL_SOURCES})
target_sources(SmartGL-convert-model PRIVATE ${SmartGL_SOURCES})
target_sources(SmartGL-convert-cubemap PRIVATE ${SmartGL_SOURCES})

set(SmartGL_LIBS "GL;GLEW;BulletDynamics;BulletCollision;LinearMath;Bullet3Common;freetype")

target_link_libraries(SmartGL ${SmartGL_LIBS})
target_link_libraries(SmartGL-convert-model GL GLEW assimp)
target_link_libraries(SmartGL-convert-cubemap GL GLEW)

target_compile_definitions(SmartGL-convert-model PRIVATE BUILD_MODEL_CONVERTER SmartGL_NO_PHYSICS NO_FREETYPE)
target_compile_definitions(SmartGL-convert-cubemap PRIVATE SmartGL_NO_PHYSICS NO_FREETYPE)

export(TARGETS SmartGL FILE "${CMAKE_SOURCE_DIR}/SmartGLConfig.cmake")

set_target_properties(SmartGL PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${SmartGL_INCLUDE_DIRS}"
  INTERFACE_LINK_LIBRARIES "${SmartGL_LIBS}"
)