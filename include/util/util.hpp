#ifndef UTIL_HPP
#define UTIL_HPP

#include <GL/glew.h>

#include "./Exception.hpp"
#include "./GL-math.hpp"

#define SmartGL_initialized GL::_util::initialized

namespace GL {

	void init(unsigned int screenWidth, unsigned int screenHeight);

	uvec2 getScreenSize();

	class _util {
	protected:

		static bool initialized;

		static unsigned int screenWidth;
		static unsigned int screenHeight;

		static const unsigned int std140arrayAlignments[];
		static const unsigned int std430arrayAlignments[];
		static const GLint colorStorageTypes[];
		static const GLint depthStencilStorageTypes[];
		static const GLenum formats[];
		static const GLenum types[];
		static const GLenum drawModes[];
		static const GLint wrapModes[];

		static GLint maxVertexAttribs;
		static GLint maxTextureUnits;
		static GLint maxImageUnits;
		static GLint maxColorTextureSamples;
		static GLint maxDepthTextureSamples;
		static GLint maxIntTextureSamples;
		static GLint maxColorAttachments;
		static GLint maxUniformBufferBindings;
		static GLint maxShaderBufferBindings;
		static GLint maxComputeWorkGroupSize[3];
		static GLint maxComputeWorkGroupCount[3];
		static GLint maxComputeWorkGroupInvocations;
		static GLuint dummyVao;

		static void* cubeMapIrradianceProgram;
		static void* cubeMapIrradianceUniforms;
		static void* cubeMapSpecularProgram;
		static void* cubeMapSpecularUniforms;
		static void* cubeMapCommonShaderLoader;

		static const char* cubeMapIrradianceProgramSource[2];
		static const char* cubeMapSpecularProgramSource[2];
		static const char* importanceSampleCommonCode;
		
		static vec3 cubeMap_lookAt_targetVectors[6];
		static vec3 cubeMap_lookAt_upVectors[6];
		static mat3 cubeMap_rotationMatrices[6];

		_util();
		_util(const _util&) = delete;
		void operator = (const _util&) = delete;
		virtual ~_util() { }

		friend void init(unsigned int screenWidth, unsigned int screenHeight);
		friend uvec2 getScreenSize();

	};

}

#endif