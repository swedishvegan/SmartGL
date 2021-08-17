#ifndef	SHADOWRENDERER_HPP
#define SHADOWRENDERER_HPP

#include "util/util.hpp"
#include "Framebuffer/Framebuffer.hpp"
#include "Framebuffer/Renderbuffer.hpp"
#include "Texture/Texture2D.hpp"
#include "Texture/TextureCubeMap.hpp"
#include "Program/Program.hpp"
#include "Uniform/UniformTable.hpp"
#include "Uniform/UniformBufferTable.hpp"
#include "Model/ModelStructs.hpp"

namespace GL {

	class ShadowRenderer : public _util {
	protected:

		static const char* shadowRenderer_vs;

		static ShaderLoader* vertShader_noAnim;
		static ShaderLoader* vertShader_anim;

		static void initializeVertShader();

	};

	class PointLightShadowRenderer : public ShadowRenderer {
	public:

		PointLightShadowRenderer(ShadowSettings settings, unsigned int lightIndex);

		void setUpFaceForDrawing(unsigned int face);

		void setUpShadersForDrawing(bool isAnimated);

		~PointLightShadowRenderer();

		static mat4 getPVMatrix(unsigned int face, vec3 lightPos, float zFar);

	private:

		ImageTextureCubeMap lightCubeMap;
		DepthStencilRenderbuffer depthBuffer;
		GLuint fbo = 0u;
		unsigned int lightIndex;

		static Program* shadowRenderer[2];
		static UniformTable* shadowRenderer_unis[2];	
		static const char* shadowRenderer_fs;

	};

	class OverheadShadowRenderer : public ShadowRenderer {
	public:

		OverheadShadowRenderer(ShadowSettings settings);

		void setUpForDrawing();

		void setUpShadersForDrawing(bool isAnimated);

		void calcPVMatrix(vec3 lightDir, BoundingBox sceneBB);

		mat4 PV;
		float farPlane;

	protected:

		Framebuffer fb;
		DepthStencilRenderTexture depthTexture;

		vec3 frustumEndpoints[5];

		static Program* shadowRenderer[2];
		static const char* shadowRenderer_fs;

	};

}

GL::ShaderLoader* GL::ShadowRenderer::vertShader_noAnim = nullptr;

GL::ShaderLoader* GL::ShadowRenderer::vertShader_anim = nullptr;

void GL::ShadowRenderer::initializeVertShader() {

	if (!vertShader_anim) {

		const char* vs_source[2];
		vs_source[1] = shadowRenderer_vs;

		vertShader_noAnim = new ShaderLoader(ShaderType::VERTEX);
		vertShader_anim = new ShaderLoader(ShaderType::VERTEX);

		vs_source[0] = "#version 430 core\n";
		vertShader_noAnim->init((char**)vs_source, 2u);

		vs_source[0] = "#version 430 core\n#define ANIMATED\n";
		vertShader_anim->init((char**)vs_source, 2u);

	}

}

GL::Program* GL::PointLightShadowRenderer::shadowRenderer[2] = { nullptr, nullptr };

GL::UniformTable* GL::PointLightShadowRenderer::shadowRenderer_unis[2] = { nullptr, nullptr };

GL::PointLightShadowRenderer::PointLightShadowRenderer(GL::ShadowSettings settings, unsigned int lightIndex) : lightCubeMap(settings.shadowCubeMapSideLength, 8u + lightIndex, ColorFormat::R), depthBuffer(settings.shadowCubeMapSideLength, settings.shadowCubeMapSideLength), lightIndex(lightIndex) {
	
	glGenFramebuffers(1, &fbo);
	lightCubeMap.setMinFilter(TextureFilter::LINEAR);
	lightCubeMap.setMagFilter(TextureFilter::LINEAR);
	lightCubeMap.setWrapMode(TextureWrap::CLAMP_TO_EDGE);

	initializeVertShader();

	if (!shadowRenderer[0]) {
		
		ShaderLoader fragShader(ShaderType::FRAGMENT);
		fragShader.init(shadowRenderer_fs, false);

		shadowRenderer[0] = new Program();
		shadowRenderer[0]->init(*vertShader_noAnim, fragShader);

		shadowRenderer[1] = new Program();
		shadowRenderer[1]->init(*vertShader_anim, fragShader);

		for (int i = 0; i < 2; i++) {

			shadowRenderer_unis[i] = new UniformTable(*shadowRenderer[i]);
			shadowRenderer_unis[i]->init("lightIdx", UniformType::UINT, 1u);

		}
		
	}

}

void GL::PointLightShadowRenderer::setUpFaceForDrawing(unsigned int face) {
	
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	lightCubeMap.bind();
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, lightCubeMap.getID(), 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer.getID());
	glViewport(0, 0, lightCubeMap.sideLength(), lightCubeMap.sideLength());
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

}

void GL::PointLightShadowRenderer::setUpShadersForDrawing(bool isAnimated) {

	shadowRenderer_unis[isAnimated]->set("lightIdx", lightIndex);
	shadowRenderer_unis[isAnimated]->update();
	shadowRenderer[isAnimated]->use();

}

GL::PointLightShadowRenderer::~PointLightShadowRenderer() { if (fbo) glDeleteFramebuffers(1, &fbo); }

GL::mat4 GL::PointLightShadowRenderer::getPVMatrix(unsigned int face, GL::vec3 lightPos, float zFar) { return perspective(radians(90.0f), 1.0f, zFar / 500.0f, zFar * 2.0f) * lookAt(lightPos, lightPos + _util::cubeMap_lookAt_targetVectors[face], _util::cubeMap_lookAt_upVectors[face]); }

GL::Program* GL::OverheadShadowRenderer::shadowRenderer[2] = { nullptr, nullptr };

GL::OverheadShadowRenderer::OverheadShadowRenderer(ShadowSettings settings) : fb(settings.overheadMapSideLength, settings.overheadMapSideLength), depthTexture(settings.overheadMapSideLength, settings.overheadMapSideLength, 14u) {
	
	fb.setDepthStencilTarget(depthTexture);

	initializeVertShader();

	if (!shadowRenderer[0]) {

		ShaderLoader fragShader(ShaderType::FRAGMENT);
		fragShader.init(shadowRenderer_fs, false);

		shadowRenderer[0] = new Program();
		shadowRenderer[0]->init(*vertShader_noAnim, fragShader);

		shadowRenderer[1] = new Program();
		shadowRenderer[1]->init(*vertShader_anim, fragShader);

	}

}

void GL::OverheadShadowRenderer::setUpForDrawing() {

	fb.useWithoutColorBuffer();
	glClear(GL_DEPTH_BUFFER_BIT);

}

void GL::OverheadShadowRenderer::setUpShadersForDrawing(bool isAnimated) { shadowRenderer[isAnimated]->use(); }

void GL::OverheadShadowRenderer::calcPVMatrix(vec3 lightDir, BoundingBox sceneBB) {
	
	vec3 center = (sceneBB.start + sceneBB.end) / 2.0f;

	float r = length(center - sceneBB.start);
	vec3 planeCenter = center - r * lightDir;
	vec3 up = normalize(orthogonalTo(lightDir));
	
	farPlane = 2.0f * r;
	PV = ortho(vec2(farPlane), farPlane) * lookAt(planeCenter, center, up);

}

const char* GL::ShadowRenderer::shadowRenderer_vs = \
\
"layout (location = 0) in vec3 inPos; \
\
\n#ifdef ANIMATED\n \
layout(location = 5) in ivec4 boneIndices; \
layout(location = 6) in vec4 boneWeights; \
\n#endif\n \
\
layout(std140, binding = 0) uniform PBR_inputs { \
	\
	mat4 model; \
	mat3 normalMatrix; \
	vec3 lightPositions[16]; \
	vec3 lightColors[16]; \
	mat4 trans; \
	vec3 camPos; \
	\
}; \
\
\n#ifdef ANIMATED\n \
layout(std140, binding = 1) uniform boneData { \
	\
	mat4 boneModelMatrices[64]; \
	mat3 boneNormalMatrices[64]; \
	\
}; \
\n#endif\n \
\
out vec3 pos; \
\
void main() { \
	\
	\n#ifdef ANIMATED\n \
	mat4 boneModelMatrix = mat4(0.0f); \
	for (uint i = 0u; i < 4u; i++) { boneModelMatrix += boneModelMatrices[boneIndices[i]] * boneWeights[i]; } \
	mat4 finalModelMatrix = model * boneModelMatrix; \
	\n#else\n \
	mat4 finalModelMatrix = model; \
	\n#endif\n \
	\
	vec4 worldSpace = finalModelMatrix * vec4(inPos, 1.0f); \
	pos = worldSpace.xyz; \
	gl_Position = trans * worldSpace; \
	\
}";

const char* GL::PointLightShadowRenderer::shadowRenderer_fs = \
\
"#version 430 core\n \
\
in vec3 pos; \
out vec4 dist; \
\
layout(std140, binding = 0) uniform PBR_inputs { \
	\
	mat4 model; \
	mat3 normalMatrix; \
	vec3 lightPositions[16]; \
	vec3 lightColors[16]; \
	mat4 trans; \
	vec3 camPos; \
	\
}; \
\
uniform uint lightIdx; \
\
void main() { dist = vec4(length(pos - lightPositions[lightIdx])); dist.w = 1.0f; }";

const char* GL::OverheadShadowRenderer::shadowRenderer_fs = "#version 430 core\n in vec3 pos; void main(){}";

#endif