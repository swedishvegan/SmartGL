#ifndef UTIL_HPP
#define UTIL_HPP

#include "util/Exception.hpp"
#include "util/GL-math.hpp"

namespace GL {

	class _util {
	protected:

		static unsigned int screenWidth;
		static unsigned int screenHeight;

		static const unsigned int std140arrayAlignments[];
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

		static mat3 cubeMap_rotationMatrices[6];
		static vec3 cubeMap_lookAt_targetVectors[6];
		static vec3 cubeMap_lookAt_upVectors[6];

		_util();
		_util(const _util&) = delete;
		void operator = (const _util&) = delete;
		virtual ~_util() { }

		friend void init(unsigned int screenWidth, unsigned int screenHeight);

	};
	
	bool initialized = false;

	void init(unsigned int screenWidth, unsigned int screenHeight);

}

void GL::init(unsigned int screenWidth, unsigned int screenHeight) {

	if (initialized) throw Exception("Cannot call init function twice.");
	if (glewInit() != GLEW_OK) throw Exception("Failed to load OpenGL functions.");
	if (screenWidth * screenHeight == 0u) throw Exception("Screen dimension passed to init function was zero.");

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glViewport(0, 0, screenWidth, screenHeight);

	GL::_util::screenWidth = screenWidth; GL::_util::screenHeight = screenHeight;
	initialized = true;

}

unsigned int GL::_util::screenWidth = 0u;
unsigned int GL::_util::screenHeight = 0u;

const unsigned int GL::_util::std140arrayAlignments[] = {

	16u, 16u, 16u, 16u, 16u, 16u, 16u, 16u, 16u, 16u, 16u, 32u, 16u, 16u, 16u, 32u,
	16u, 16u, 16u, 32u, 16u, 32u, 16u, 16u, 16u, 16u, 16u, 32u, 16u, 32u, 16u, 32u, 16u, 32u

};

const GLint GL::_util::colorStorageTypes[] = {

		GL_R16F, GL_RG16F, GL_RGB16F, GL_RGBA16F,
		GL_R32F, GL_RG32F, GL_RGB32F, GL_RGBA32F,
		GL_R8I, GL_RG8I, GL_RGB8I, GL_RGBA8I,
		GL_R16I, GL_RG16I, GL_RGB16I, GL_RGBA16I,
		GL_R32I, GL_RG32I, GL_RGB32I, GL_RGBA32I,
		GL_R8UI, GL_RG8UI, GL_RGB8UI, GL_RGBA8UI,
		GL_R16UI, GL_RG16UI, GL_RGB16UI, GL_RGBA16UI,
		GL_R32UI, GL_RG32UI, GL_RGB32UI, GL_RGBA32UI

};

const GLint GL::_util::depthStencilStorageTypes[] = { GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT16, GL_DEPTH32F_STENCIL8, GL_DEPTH24_STENCIL8 };

const GLenum GL::_util::formats[] = {

			GL_RED, GL_RG, GL_RGB, GL_RGBA,
			GL_RED_INTEGER, GL_RG_INTEGER, GL_RGB_INTEGER, GL_RGBA_INTEGER

};

const GLenum GL::_util::types[] = { GL_FLOAT, GL_BYTE, GL_SHORT, GL_INT, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT };

const GLenum GL::_util::drawModes[] = { GL_POINTS, GL_LINES, GL_TRIANGLES, GL_LINE_STRIP, GL_LINE_LOOP, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN };

const GLint GL::_util::wrapModes[] = { GL_REPEAT, GL_MIRRORED_REPEAT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER };

GLint GL::_util::maxVertexAttribs = 0;
GLint GL::_util::maxTextureUnits = 0;
GLint GL::_util::maxImageUnits = 0;
GLint GL::_util::maxColorTextureSamples = 0;
GLint GL::_util::maxDepthTextureSamples = 0;
GLint GL::_util::maxIntTextureSamples = 0;
GLint GL::_util::maxColorAttachments = 0;
GLint GL::_util::maxUniformBufferBindings = 0;
GLint GL::_util::maxShaderBufferBindings = 0;
GLint GL::_util::maxComputeWorkGroupSize[3] = { 0, 0, 0 };
GLint GL::_util::maxComputeWorkGroupCount[3] = { 0, 0, 0 };
GLint GL::_util::maxComputeWorkGroupInvocations = 0;
GLuint GL::_util::dummyVao = 0;

void* GL::_util::cubeMapIrradianceProgram = nullptr;
void* GL::_util::cubeMapIrradianceUniforms = nullptr;
void* GL::_util::cubeMapSpecularProgram = nullptr;
void* GL::_util::cubeMapSpecularUniforms = nullptr;
void* GL::_util::cubeMapCommonShaderLoader = nullptr;

GL::mat3 GL::_util::cubeMap_rotationMatrices[6] = {

	GL::mat3(
		0.0f, 0.0f, -1.0f,
		0.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f
	),

	GL::mat3(
		0.0f, 0.0f, 1.0f,
		0.0f, -1.0f, 0.0f,
		-1.0f, 0.0f, 0.0f
	),

	GL::mat3(
		1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f
	),

	GL::mat3(
		1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, -1.0f, 0.0f
	),

	GL::mat3(
		1.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, 0.0f, 1.0f
	),

	GL::mat3(
		-1.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, 0.0f, -1.0f
	)

};

GL::vec3 GL::_util::cubeMap_lookAt_targetVectors[6] = {

	GL::vec3(1.0f, 0.0f, 0.0f),
	GL::vec3(-1.0f, 0.0f, 0.0f),
	GL::vec3(0.0f, 1.0f, 0.0f),
	GL::vec3(0.0f, -1.0f, 0.0f),
	GL::vec3(0.0f, 0.0f, 1.0f),
	GL::vec3(0.0f, 0.0f, -1.0f)

};

GL::vec3 GL::_util::cubeMap_lookAt_upVectors[6] = {

	GL::vec3(0.0f, -1.0f, 0.0f),
	GL::vec3(0.0f, -1.0f, 0.0f),
	GL::vec3(0.0f, 0.0f, 1.0f),
	GL::vec3(0.0f, 0.0f, -1.0f),
	GL::vec3(0.0f, -1.0f, 0.0f),
	GL::vec3(0.0f, -1.0f, 0.0f)

};

GL::_util::_util() {
	
	if (!initialized) throw Exception("A GL object cannot be created before the init function has been called.");
	else if (maxVertexAttribs == 0) {

		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
		glGetIntegerv(GL_MAX_IMAGE_UNITS, &maxImageUnits);
		glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &maxColorTextureSamples);
		glGetIntegerv(GL_MAX_DEPTH_TEXTURE_SAMPLES, &maxDepthTextureSamples);
		glGetIntegerv(GL_MAX_INTEGER_SAMPLES, &maxIntTextureSamples);
		glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColorAttachments);
		glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &maxUniformBufferBindings);
		glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &maxShaderBufferBindings);
		for (unsigned int i = 0u; i < 3u; i++) glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &maxComputeWorkGroupSize[i]);
		for (unsigned int i = 0u; i < 3u; i++) glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &maxComputeWorkGroupCount[i]);
		glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &maxComputeWorkGroupInvocations);
		glGenVertexArrays(1, &dummyVao);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		const GLubyte* version = glGetString(GL_VERSION);
		if (!version) throw Exception("Failed to check OpenGL version.");
		if (version[0] != '4') throw Exception("OpenGL version must be 4.3 or greater.");
		if (!version[1]) throw Exception("Failed to check OpenGL minor version.");
		if (!version[2]) throw Exception("Failed to check OpenGL minor version.");
		if ((int)version[2] - (int)'3' < 0) throw Exception("OpenGL version must be 4.3 or greater.");

	}

}

const char* GL::_util::cubeMapIrradianceProgramSource[] = {
	
	"#version 430 core\n \
	\
	const vec2 coords[6] = vec2[]( \
		vec2(-1.0f, -1.0f), \
		vec2(1.0f, -1.0f), \
		vec2(1.0f, 1.0f), \
		vec2(1.0f, 1.0f), \
		vec2(-1.0f, 1.0f), \
		vec2(-1.0f, -1.0f) \
	); \
	\
	uniform mat3 rot; \
	\
	out vec3 ray; \
	\
	void main() { \
		\
		gl_Position = vec4(coords[gl_VertexID], 0.0f, 1.0f); \
		ray.xy = coords[gl_VertexID]; \
		ray.z = 1.0f; \
		ray = rot * ray; \
		\
	}",

	"#version 430 core\n \
	\
	in vec3 ray; \
	\
	uniform samplerCube inputCubeMap; \
	\
	layout(location = 0) out vec4 fragColor; \
	\
	const float PI = 3.14159265359f; \
	const float sampleDelta = 0.1f; \
	\
	void main() { \
		\
		vec3 r = normalize(ray); \
		vec3 irradiance = vec3(0.0f); \
		\
		vec3 up = (r.y < 0.99f) ? vec3(0.0f, 1.0f, 0.0f) : vec3(0.0f, 0.0f, 1.0f); \
		vec3 right = normalize(cross(up, r)); \
		up = normalize(cross(r, right)); \
		mat3 transformation = mat3(right, up, r); \
		\
		uint numSamples = 0u; \
		\
		for (float phi = 0.0f; phi < 2.0f * PI; phi += sampleDelta) \
			for (float theta = 0.0f; theta < 0.5f * PI; theta += sampleDelta) { \
				\
				float cosTheta = cos(theta); \
				float sinTheta = sin(theta); \
				\
				vec3 tangentSample = vec3(sinTheta * cos(phi),  sinTheta * sin(phi), cosTheta); \
				vec3 sampleVec = transformation * tangentSample; \
				vec3 sampleColor = pow(texture(inputCubeMap, sampleVec).rgb, vec3(2.2f)); \
				\
				irradiance += sampleColor * cosTheta * sinTheta; \
				numSamples++; \
				\
			} \
		\
		irradiance *= PI / float(numSamples); \
		fragColor = vec4(irradiance, 1.0f); \
		\
	}"

};

const char* GL::_util::cubeMapSpecularProgramSource[] = {

	"#version 430 core\n \
	\
	in vec3 ray; \
	\
	uniform float roughness; \
	uniform samplerCube inputCubeMap; \
	\
	layout(location = 0) out vec4 fragColor;",

	"void main() { \
		\
		vec3 r = normalize(ray); \
		float totalWeight = 0.0f; \
		vec3 prefilteredColor = vec3(0.0f); \
		\
		vec3 up = (r.y < 0.99f) ? vec3(0.0f, 1.0f, 0.0f) : vec3(0.0f, 0.0f, 1.0f); \
		vec3 right = normalize(cross(up, r)); \
		up = normalize(cross(r, right)); \
		mat3 transformation = mat3(right, up, r); \
		\
		for (uint i = 0u; i < numSamples; i++) { \
			\
			vec2 Xi = Hammersley(i); \
			vec3 H = ImportanceSampleGGX(Xi, transformation, roughness); \
			vec3 L = normalize(2.0f * dot(r, H) * H - r); \
			\
			float NdotL = max(0.0f, dot(r, L)); \
			if (NdotL > 0.0f) { \
				\
				prefilteredColor += pow(texture(inputCubeMap, L).rgb, vec3(2.2f)) * NdotL; \
				totalWeight += NdotL; \
				\
			} \
			\
		} \
		\
		if (totalWeight > 0.0f) prefilteredColor /= totalWeight; \
		fragColor = vec4(prefilteredColor, 1.0f); \
		\
	}"

};

const char* GL::_util::importanceSampleCommonCode = \
\
"const float PI = 3.14159265359f; \
const uint numSamples = 512u; \
\
float RadicalInverse_VdC(uint bits) { \
	\
	bits = (bits << 16u) | (bits >> 16u); \
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u); \
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u); \
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u); \
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u); \
	return float(bits) * 2.3283064365386963e-10f; \
	\
} \
\
vec2 Hammersley(uint i) { return vec2(float(i) / float(numSamples), RadicalInverse_VdC(i)); } \
\
vec3 ImportanceSampleGGX(vec2 Xi, mat3 transformation, float roughness) { \
	\
	float a = roughness * roughness; \
	\
	float phi = 2.0f * PI * Xi.x; \
	float cosTheta = sqrt((1.0f - Xi.y) / (1.0f + (a * a - 1.0f) * Xi.y)); \
	float sinTheta = sqrt(1.0f - cosTheta * cosTheta); \
	\
	vec3 H = vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta); \
	return normalize(transformation * H); \
	\
}";

#endif