#ifndef SMARTGL_HPP
#define SMARTGL_HPP

#define GLEW_STATIC
#include <GL/glew.h>

#include "Font/FontLoader.hpp"
#include "Font/Font.hpp"

#include "Framebuffer/Framebuffer.hpp"
#include "Framebuffer/Renderbuffer.hpp"
#include "Framebuffer/RenderTexture.hpp"

#include "Model/Model.hpp"
#include "Model/Scene.hpp"

#ifndef SmartGL_NO_PHYSICS
#include "Model/PhysicsModel.hpp"
#endif

#include "Program/Program.hpp"
#include "Program/ShaderLoader.hpp"

#include "Texture/Image.hpp"
#include "Texture/Texture2D.hpp"
#include "Texture/Texture3D.hpp"
#include "Texture/TextureCubeMap.hpp"
#include "Texture/TextureConverter.hpp"

#include "Uniform/UniformBufferTable.hpp"
#include "Uniform/ShaderStorageBufferTable.hpp"
#include "Uniform/UniformTable.hpp"

#include "VertexArray/VertexArray.hpp"

#endif