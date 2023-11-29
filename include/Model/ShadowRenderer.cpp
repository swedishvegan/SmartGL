
#include "./ShadowRenderer.hpp"

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
			shadowRenderer_unis[i]->init("lightIdx", UniformType::UINT, 1u, "isInstanced", UniformType::INT, 1u);

		}
		
	}

}

void GL::PointLightShadowRenderer::bindTextures() { lightCubeMap.bind(); }

void GL::PointLightShadowRenderer::setUpFaceForDrawing(unsigned int face) {
	
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	lightCubeMap.bind();
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, lightCubeMap.getID(), 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer.getID());
	glViewport(0, 0, lightCubeMap.sideLength(), lightCubeMap.sideLength());
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

}

void GL::PointLightShadowRenderer::setUpShadersForDrawing(bool isAnimated, bool isInstanced) {

	shadowRenderer_unis[isAnimated]->set("lightIdx", lightIndex);
	shadowRenderer_unis[isAnimated]->set("isInstanced", (isInstanced) ? 1 : 0);
	shadowRenderer_unis[isAnimated]->update();
	shadowRenderer[isAnimated]->use();

}

GL::PointLightShadowRenderer::~PointLightShadowRenderer() { if (fbo) glDeleteFramebuffers(1, &fbo); }

GL::mat4 GL::PointLightShadowRenderer::getPVMatrix(unsigned int face, GL::vec3 lightPos, float zFar) { return perspective(radians(90.0f), 1.0f, zFar / 500.0f, zFar * 2.0f) * lookAt(lightPos, lightPos + _util::cubeMap_lookAt_targetVectors[face], _util::cubeMap_lookAt_upVectors[face]); }

GL::Program* GL::OverheadShadowRenderer::shadowRenderer[2] = { nullptr, nullptr };

GL::UniformTable* GL::OverheadShadowRenderer::shadowRenderer_unis[2] = { nullptr, nullptr };

GL::OverheadShadowRenderer::OverheadShadowRenderer(ShadowSettings settings) : fb(settings.overheadMapSideLength, settings.overheadMapSideLength), depthTexture(settings.overheadMapSideLength, settings.overheadMapSideLength, 14u) {
	
	fb.setDepthStencilTarget(depthTexture);
	if (settings.overheadShadowFancy) {

		outerDepthTexture = new DepthStencilRenderTexture(settings.overheadMapSideLength, settings.overheadMapSideLength, 15u);
		outerFb = new Framebuffer(settings.overheadMapSideLength, settings.overheadMapSideLength);
		outerFb->setDepthStencilTarget(*outerDepthTexture);

	}
	
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
			shadowRenderer_unis[i]->init("isInstanced", UniformType::INT, 1u);

		}

	}

}

bool GL::OverheadShadowRenderer::isFancy() { return outerFb; }

void GL::OverheadShadowRenderer::bindTextures() {

	depthTexture.bind();
	if (outerFb) outerDepthTexture->bind();

}

void GL::OverheadShadowRenderer::setUpForDrawing(bool outerBuffer) {
	
	if (outerBuffer) outerFb->useWithoutColorBuffer();
	else fb.useWithoutColorBuffer();

	glClear(GL_DEPTH_BUFFER_BIT);

}

void GL::OverheadShadowRenderer::setUpShadersForDrawing(bool isAnimated, bool isInstanced) { 
	
	shadowRenderer_unis[isAnimated]->set("isInstanced", (isInstanced) ? 1 : 0);
	shadowRenderer_unis[isAnimated]->update();
	shadowRenderer[isAnimated]->use();

}

void GL::OverheadShadowRenderer::calcPVMatrix(vec3 lightDir, BoundingBox sceneBB, BoundingBox* outerBB, bool isOuter) {
	
	vec3 center = (sceneBB.start + sceneBB.end) / 2.0f;

	float r = length(center - sceneBB.start);
	float r_depth = (outerBB && !isOuter) ? length(outerBB->end - outerBB->start) / 2.0f : r;
	vec3 planeCenter = center - r_depth * lightDir;
	vec3 up = normalize(orthogonalTo(lightDir));
	r *= 2.0f; r_depth *= 2.0f;
	
	if (isOuter) {

		outerFarPlane = r_depth;
		outerPV = ortho(vec2(r), r_depth) * lookAt(planeCenter, center, up);

	}
	else {

		farPlane = r_depth;
		PV = ortho(vec2(r), r_depth) * lookAt(planeCenter, center, up);

	}

}

GL::OverheadShadowRenderer::~OverheadShadowRenderer() { if (outerFb) { delete outerDepthTexture; delete outerFb; }; }

const char* GL::ShadowRenderer::shadowRenderer_vs = \
\
"layout (location = 0) in vec3 inPos; \
\
\n#ifdef ANIMATED\n \
layout(location = 5) in ivec4 boneIndices; \
layout(location = 6) in vec4 boneWeights; \
\n#endif\n \
\
layout(std430, binding = 0) buffer modelMatrixInstances { mat4 modelMatrices[]; }; \
\
layout(std140, binding = 0) uniform PBR_inputs { \
	\
	mat4 modelMat; \
	mat3 normalMat; \
	vec3 lightPositions[16]; \
	vec3 lightColors[16]; \
	mat4 trans; \
	vec3 camPos; \
	\
}; \
\
uniform int isInstanced; \
mat4 modelMatrix = (isInstanced == 1) ? modelMatrices[gl_InstanceID] : modelMat; \
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
	mat4 finalModelMatrix = modelMatrix * boneModelMatrix; \
	\n#else\n \
	mat4 finalModelMatrix = modelMatrix; \
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
	mat4 modelMat; \
	mat3 normalMat; \
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