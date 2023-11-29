
#include "./Scene.hpp"

GL::Texture2D* GL::Scene::texBRDF = nullptr;
GL::Program* GL::Scene::progBRDF = nullptr;
GL::Program* GL::Scene::progScene = nullptr;
GL::Program* GL::Scene::progLights = nullptr;
GL::UniformTable* GL::Scene::progLightsUnis = nullptr;
GL::Program* GL::Scene::progSkybox = nullptr;
GLint GL::Scene::progSkybox_bgBrightness = -1;
GL::UniformBufferTable* GL::Scene::raytraceUnis = nullptr;

#define _GL_Scene_initializers(sw, sh, smd) \
fbColor(sw, sh, 0u, true, ColorFormat::RGBA, DataType::F16), \
fbDepth(sw, sh, 0u, true, DepthStencilFormat::DEPTH_32), \
fb(sw, sh), \
specularDim(smd)

GL::Scene::Scene(float zNear, float zFar, GL::ShadowSettings shadowSettings) : _GL_Scene_initializers(_util::screenWidth, _util::screenHeight, 0u) { init(nullptr, zNear, zFar, shadowSettings); }

GL::Scene::Scene(ImageTextureCubeMap& background, float zNear, float zFar, GL::ShadowSettings shadowSettings, unsigned int specularMapDetail) : _GL_Scene_initializers(_util::screenWidth, _util::screenHeight, specularMapDetail) { init(&background, zNear, zFar, shadowSettings); }

GL::Scene::Scene(GL::uvec2 screenSize, float zNear, float zFar, GL::ShadowSettings shadowSettings) : _GL_Scene_initializers(screenSize.x, screenSize.y, 0u) { init(nullptr, zNear, zFar, shadowSettings); }

GL::Scene::Scene(GL::uvec2 screenSize, ImageTextureCubeMap& background, float zNear, float zFar, GL::ShadowSettings shadowSettings, unsigned int specularMapDetail) : _GL_Scene_initializers(screenSize.x, screenSize.y, specularMapDetail) { init(&background, zNear, zFar, shadowSettings); }


void GL::Scene::addModel(Drawable& model, bool use, bool castsShadow, SampleSettings sampleSettings) {
	
	for (unsigned int i = 0u; i < models.size(); i++) if (&model == models[i]) return;

	models.push_back(&model);
	isModelUsed.push_back(use);
	modelCastsShadow.push_back(castsShadow);
	modelSettings.push_back(sampleSettings);

}

void GL::Scene::useModel(GL::Drawable& model, bool use) {

	unsigned int idx = models.size();
	for (unsigned int i = 0u; i < models.size(); i++) if (&model == models[i]) { idx = i; break; }

	if (idx < models.size()) isModelUsed[idx] = use;

}

void GL::Scene::setModelShouldCastShadow(GL::Drawable& model, bool castsShadow) {

	unsigned int idx = models.size();
	for (unsigned int i = 0u; i < models.size(); i++) if (&model == models[i]) { idx = i; break; }

	if (idx < models.size()) modelCastsShadow[idx] = castsShadow;

}

void GL::Scene::applySampleSettings(GL::Drawable& model, GL::SampleSettings sampleSettings) {
	
	unsigned int idx = models.size();
	for (unsigned int i = 0u; i < models.size(); i++) if (&model == models[i]) { idx = i; break; }

	if (idx < models.size()) modelSettings[idx] = sampleSettings;

}

void GL::Scene::draw() { draw_commonCode(nullptr); }

void GL::Scene::draw(GL::Framebuffer& fb) { draw_commonCode(&fb); }

bool GL::Scene::hasBackground() const { return bg; }

void GL::Scene::setLightPosition(unsigned int index, GL::vec3 position) { lightPositions[index % 16u] = position; }

void GL::Scene::setLightColor(unsigned int index, GL::vec3 color) { lightColors[index % 16u] = max(color, 0.0f); }

void GL::Scene::setBackgroundColor(GL::vec3 color) {

	color = max(color, 0.0f);
	bgColor = color;

}

void GL::Scene::setBackgroundBrightness(float brightness) { bgBrightness = max(brightness, 0.0f); }

void GL::Scene::setOverheadLightDirection(GL::vec3 direction) { if (dot(direction, direction) == 0.0f) throw Exception("A scene's light direction cannot be zero."); lightDirection = normalize(direction); }

void GL::Scene::setOverheadLightColor(GL::vec3 color) { lightColor = max(color, 0.0f); }

GL::vec3 GL::Scene::getLightPosition(unsigned int index) const { return lightPositions[index % 16u]; }

GL::vec3 GL::Scene::getLightColor(unsigned int index) const { return lightColors[index % 16u]; }

GL::vec3 GL::Scene::getBackgroundColor() const { return bgColor; }

float GL::Scene::getBackgroundBrightness() const { return bgBrightness; }

GL::vec3 GL::Scene::getOverheadLightDirection() const { return lightDirection; }

GL::vec3 GL::Scene::getOverheadLightColor() const { return lightColor; }

GL::vec3 GL::Scene::getCameraPosition() const { return camPos; }

GL::mat4 GL::Scene::getPerspectiveMatrix() const { return perspectiveMatrix; }

GL::mat4 GL::Scene::getOverheadShadowPVMatrix() const { return (overheadShadowRenderer) ? overheadShadowRenderer->PV : mat4(); }

float GL::Scene::getOverheadShadowFarPlane() const { return (overheadShadowRenderer) ? overheadShadowRenderer->farPlane : 0.0f; }

GL::BoundingBox GL::Scene::getOverheadShadowBoundingBox() const { return bb; }

GL::mat4 GL::Scene::getOverheadOuterShadowPVMatrix() const { return (overheadShadowRenderer) ? overheadShadowRenderer->outerPV : mat4(); }

float GL::Scene::getOverheadOuterShadowFarPlane() const { return (overheadShadowRenderer) ? overheadShadowRenderer->outerFarPlane : 0.0f; }

GL::BoundingBox GL::Scene::getOverheadOuterShadowBoundingBox() const { return outerBb; }

GL::BoundingBox GL::Scene::getBoundingBoxApproximation() {

	BoundingBox bbApprox;
	bool firstModel = true;

	for (unsigned int i = 0u; i < models.size(); i++) if (isModelUsed[i]) {

		if (firstModel) {

			bbApprox = models[i]->getWorldSpaceBoundingBoxApproximation();
			firstModel = false;

		}
		else bbApprox = bbApprox + models[i]->getWorldSpaceBoundingBoxApproximation();

	}
	
	return bbApprox;

}

bool GL::Scene::usesFancyOverheadShadows() const { return (overheadShadowRenderer) ? overheadShadowRenderer->isFancy() : false; }

unsigned int GL::Scene::getNumPointLights() const { return numPointLights; }

void GL::Scene::updateCamera(GL::BoundingBox sceneBB, GL::vec3 cameraPosition, GL::vec3 lookingAt, GL::vec3 up, float FoV) { updateCamera_commonCode(sceneBB, nullptr, cameraPosition, lookingAt, up, FoV); }

void GL::Scene::updateCamera(GL::BoundingBox closeBB, GL::BoundingBox farBB, GL::vec3 cameraPosition, GL::vec3 lookingAt, GL::vec3 up, float FoV) { updateCamera_commonCode(closeBB, &farBB, cameraPosition, lookingAt, up, FoV); }

void GL::Scene::drawBackground() {

	if (!cameraUpdated) throw Exception("A scene cannot be rendered until the camera has been set.");
	if (backgroundUsed) return;

	vec3 clearColor = bgColor * bgBrightness;
	clearColor /= clearColor + 1.0f;
	clearColor = pow(clearColor, 1.0f / 2.2f);

	fb.use();
	glClearColor(clearColor.x, clearColor.y, clearColor.z, 1.0f);
	Render::clearBuffers(COLOR_BUFFER | DEPTH_BUFFER);

	if (bg) {

		bg->bind();
		raytraceUnis->bind();
		progSkybox->use();
		glUniform1f(progSkybox_bgBrightness, bgBrightness);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glClear(GL_DEPTH_BUFFER_BIT);

	}

	backgroundUsed = true;

}

void GL::Scene::drawToFramebuffer(GL::Framebuffer* fb) {

#define _GL_Scene_fbSizeCheck(w1, h1, w2, h2, em1, em2) \
{ if (w1 != w2 || h1 != h2) throw Exception(em1 + std::to_string(w1) + ", " + std::to_string(h1) + em2 + std::to_string(w2) + ", " + std::to_string(h2) + ")."); }

	if (fb) _GL_Scene_fbSizeCheck(fb->getWidth(), fb->getHeight(), fbColor.width(), fbColor.height(), "Output framebuffer's size (", ") must match scene internal framebuffer' size (")
	else _GL_Scene_fbSizeCheck(_util::screenWidth, _util::screenHeight, fbColor.width(), fbColor.height(), "Default framebuffer's dimensions (", ") do not match scene internal framebuffer's dimensions (");
	
	if (!backgroundUsed) drawBackground();

	progScene->use();
	if (fb) fb->use();
	else Framebuffer::useDefault();
	Render::clearBuffers(COLOR_BUFFER | DEPTH_BUFFER);
	fbColor.bind();
	glDrawArrays(GL_TRIANGLES, 0, 6);
	
	for (int i = 0; i < 16; i++) {
		
		vec3 col = lightColors[i];
		float scale = max(col.x, max(col.y, col.z));
		col /= scale;

		progLightsUnis->setElement("lightPositions", i, lightPositions[i]);
		progLightsUnis->setElement("lightColors", i, col);
		progLightsUnis->setElement("lightStrengths", i, scale);

	}
	progLightsUnis->set<vec3>("camPos", camPos);
	progLightsUnis->update();

	progLights->use();
	raytraceUnis->bind();
	fbDepth.bind();
	glDrawArrays(GL_TRIANGLES, 0, 6);
	
	backgroundUsed = false;

}

void GL::Scene::use() {
	
	if (alreadyUsing) return;

	if (bg) irradianceMap->bind();
	if (bg) specularMap->bind();
	texBRDF->bind();
	
	for (unsigned int i = 0u; i < numPointLights; i++) pointLightShadowRenderers[i]->bindTextures();
	if (overheadShadowRenderer) overheadShadowRenderer->bindTextures();

	fb.use();

	alreadyUsing = true;

}

GL::Scene::~Scene() {

	if (bg) {
		
		delete irradianceMap;
		delete specularMap;
		
	}

	if (numPointLights) {

		for (unsigned int i = 0u; i < numPointLights; i++) delete pointLightShadowRenderers[i];
		delete[] pointLightShadowRenderers;

	}

	if (overheadShadowRenderer) delete overheadShadowRenderer;

}

void GL::Scene::init(ImageTextureCubeMap* background, float zNear, float zFar, GL::ShadowSettings shadowSettings) {

	if (shadowSettings.numLights > 6u) throw Exception("A shadow renderer must have between 1 and 6 lights, not " + std::to_string(shadowSettings.numLights) + ".");

	if (!texBRDF) {

		texBRDF = new Texture2D(512u, 512u, 5u, ColorFormat::RG, DataType::F16);
		progBRDF = new Program();

		ShaderLoader vertShader(ShaderType::VERTEX);
		vertShader.init(progBRDF_source[0], false);

		ShaderLoader fragShader(ShaderType::FRAGMENT);
		fragShader.init(
			progBRDF_source[1], false,
			_util::importanceSampleCommonCode, false,
			progBRDF_source[2], false
		);

		progBRDF->init(vertShader, fragShader);
		progBRDF->use();

		Framebuffer fbBRDF(512u, 512u);
		fbBRDF.setColorTarget(*texBRDF);
		fbBRDF.use();

		texBRDF->bind();

		glBindVertexArray(_util::dummyVao);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		ShaderLoader vertShaderScene(ShaderType::VERTEX);
		vertShaderScene.init(progScene_source[0], false);

		ShaderLoader fragShaderScene(ShaderType::FRAGMENT);
		fragShaderScene.init(progScene_source[1], false);

		progScene = new Program();
		progScene->init(vertShaderScene, fragShaderScene);

		progScene->use();
		GLint loc = glGetUniformLocation(progScene->getID(), "tex");
		glUniform1i(loc, 0);

		ShaderLoader vertShaderLights(ShaderType::VERTEX);
		vertShaderLights.init(progLights_source[0], false);

		ShaderLoader fragShaderLights(ShaderType::FRAGMENT);
		fragShaderLights.init(progLights_source[1], false);

		progLights = new Program();
		progLights->init(vertShaderLights, fragShaderLights);

		progLightsUnis = new UniformTable(*progLights);
		progLightsUnis->init(
			"lightPositions", UniformType::VEC3, 16,
			"lightColors", UniformType::VEC3, 16,
			"lightStrengths", UniformType::FLOAT, 16,
			"camPos", UniformType::VEC3, 1,
			"zFar", UniformType::FLOAT, 1,
			"depthSampler", UniformType::INT, 1
		);
		progLightsUnis->set("zFar", zFar);
		progLightsUnis->set<int>("depthSampler", 0);

		ShaderLoader vertShaderSkybox(ShaderType::VERTEX);
		vertShaderSkybox.init(progSkybox_source[0], false);

		ShaderLoader fragShaderSkybox(ShaderType::FRAGMENT);
		fragShaderSkybox.init(progSkybox_source[1], false);

		progSkybox = new Program();
		progSkybox->init(vertShaderSkybox, fragShaderSkybox);

		progSkybox->use();
		loc = glGetUniformLocation(progSkybox->getID(), "skybox");
		progSkybox_bgBrightness = glGetUniformLocation(progSkybox->getID(), "bgBrightness");
		glUniform1i(loc, 0);

		raytraceUnis = new UniformBufferTable(2u);
		raytraceUnis->init(
			"start", UniformType::VEC2, 1,
			"end", UniformType::VEC2, 1,
			"sz", UniformType::FLOAT, 1,
			"rot", UniformType::MAT3, 1
		);
		raytraceUnis->set<float>("sz", -zNear);

	}

	fb.setColorTarget(fbColor);
	fb.setDepthStencilTarget(fbDepth);
	
	bg = background;
	if (bg) {
		
		irradianceMap = new IrradianceCubeMap<unsigned char>(*bg);
		specularMap = new SpecularCubeMap<unsigned char>(*bg, 4u, specularDim);

	}

	this->zNear = zNear;
	this->zFar = zFar;
	this->aspectRatio = (float)fbColor.width() / (float)fbColor.height();

	for (int i = 0; i < 16; i++) {

		lightPositions[i] = 0.0f;
		lightColors[i] = 0.0f;

	}

	numPointLights = shadowSettings.numLights;
	if (numPointLights) {

		pointLightShadowRenderers = new PointLightShadowRenderer*[numPointLights];
		for (unsigned int i = 0u; i < numPointLights; i++) pointLightShadowRenderers[i] = new PointLightShadowRenderer(shadowSettings, i);

	}

	if (shadowSettings.hasOverheadShadow) overheadShadowRenderer = new OverheadShadowRenderer(shadowSettings);

}

void GL::Scene::updateCamera_commonCode(BoundingBox closeBB, BoundingBox* farBB, vec3 cameraPosition, vec3 lookingAt, vec3 up, float FoV) {
	
	perspectiveMatrix = perspective(FoV, aspectRatio, zNear, zFar) * lookAt(cameraPosition, lookingAt, up);
	camPos = cameraPosition;

	vec2 screenDims = getScreenDims(FoV, aspectRatio, zNear) / 2.0f;
	raytraceUnis->set<vec2>("start", 0.0f - screenDims);
	raytraceUnis->set<vec2>("end", screenDims);
	raytraceUnis->set<mat3>("rot", cameraRotation(lookingAt - cameraPosition, up));
	raytraceUnis->update();

	if (overheadShadowRenderer) {

		overheadShadowRenderer->calcPVMatrix(lightDirection, closeBB, farBB, false);
		if (overheadShadowRenderer->isFancy()) {

			if (farBB) overheadShadowRenderer->calcPVMatrix(lightDirection, *farBB, nullptr, true);
			else throw Exception("A scene cannot render fancy overhead shadows unless a close and far bounding box have been given.");

		}
		else if (farBB) throw Exception("A scene without fancy overhead shadows must only have one bounding box.");

	}
	
	bb = closeBB;
	if (farBB) outerBb = *farBB;

	cameraUpdated = true;

}

void GL::Scene::draw_commonCode(Framebuffer* fb) {

	for (unsigned int i = 0u; i < numPointLights; i++) for (unsigned int face = 0u; face < 6u; face++) {

		pointLightShadowRenderers[i]->setUpFaceForDrawing(face);
		mat4 PV = PointLightShadowRenderer::getPVMatrix(face, lightPositions[i], zFar);

		for (unsigned int j = 0u; j < models.size(); j++) if (isModelUsed[j] && modelCastsShadow[j]) {

			pointLightShadowRenderers[i]->setUpShadersForDrawing(models[j]->isAnimated(), models[j]->isInstanced());
			models[j]->drawShadow(PV, models[j]->getModelMatrix(), *this);

		}

	}

	if (overheadShadowRenderer) {

#define _GL_Scene_drawOverheadShadow(pvmatrix, outerBuffer) { \
	overheadShadowRenderer->setUpForDrawing(outerBuffer); \
	\
	for (unsigned int i = 0u; i < models.size(); i++) if (isModelUsed[i] && modelCastsShadow[i]) { \
		\
		overheadShadowRenderer->setUpShadersForDrawing(models[i]->isAnimated(), models[i]->isInstanced()); \
		models[i]->drawShadow(pvmatrix, models[i]->getModelMatrix(), *this); \
		\
	} \
}

		_GL_Scene_drawOverheadShadow(overheadShadowRenderer->PV, false);
		if (overheadShadowRenderer->isFancy()) _GL_Scene_drawOverheadShadow(overheadShadowRenderer->outerPV, true);

	}

	for (unsigned int i = 0u; i < models.size(); i++) if (isModelUsed[i]) models[i]->draw(*this, modelSettings[i]);
	drawToFramebuffer(fb);
	alreadyUsing = false;

}

const char* GL::Scene::progBRDF_source[] = {

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
	out vec2 texCoords; \
	\
	void main() { \
		\
		gl_Position = vec4(coords[gl_VertexID], 0.0f, 1.0f); \
		texCoords = coords[gl_VertexID] * 0.5f + 0.5f; \
		\
	}",

	"#version 430 core\n \
	\
	in vec2 texCoords; \
	\
	layout(location = 0) out vec4 fragColor; \
	\
	const uint numSamples = 512u;",

	"float GeometrySchlickGGX(float NdotV, float roughness) { \
		\
		float a = roughness; \
		float k = (a * a) / 2.0f; \
		\
		float num = NdotV; \
		float denom = NdotV * (1.0f - k) + k; \
		\
		return num / denom; \
		\
	} \
	\
	vec2 IntegrateBRDF(float NdotV, float roughness) { \
		\
		vec3 V; \
		V.x = sqrt(1.0f - NdotV * NdotV); \
		V.y = 0.0f; \
		V.z = NdotV; \
		\
		float A = 0.0f; \
		float B = 0.0f; \
		\
		mat3 transformation = mat3(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f); \
		\
		for (uint i = 0u; i < numSamples; i++) { \
			\
			vec2 Xi = Hammersley(i); \
			vec3 H = ImportanceSampleGGX(Xi, transformation, roughness); \
			vec3 L = normalize(2.0f * dot(V, H) * H - V); \
			\
			float NdotL = clamp(L.z, 0.0f, 1.0f); \
			float NdotH = clamp(H.z, 0.0f, 1.0f); \
			float VdotH = clamp(dot(V, H), 0.0f, 1.0f); \
			\
			if (NdotL > 0.0f) { \
				\
				float ggx2 = GeometrySchlickGGX(NdotV, roughness); \
				float ggx1 = GeometrySchlickGGX(NdotL, roughness); \
				float G = ggx1 * ggx2; \
				float G_Vis = (G * VdotH) / (NdotH * NdotV); \
				float Fc = pow(1.0f - VdotH, 5.0f); \
				\
				A += (1.0f - Fc) * G_Vis; \
				B += Fc * G_Vis; \
				\
			} \
			\
		} \
		\
		A /= float(numSamples); \
		B /= float(numSamples); \
		return vec2(A, B); \
		\
	} \
	\
	void main() { fragColor = vec4(IntegrateBRDF(texCoords.x, texCoords.y), 0.0f, 1.0f); }"

};

const char* GL::Scene::progScene_source[2] = {

	"#version 430 core\n \
	\
	const vec2 verts[6] = vec2[]( \
		vec2(0.0f, 0.0f), \
		vec2(1.0f, 0.0f), \
		vec2(1.0f, 1.0f), \
		vec2(1.0f, 1.0f), \
		vec2(0.0f, 1.0f), \
		vec2(0.0f, 0.0f) \
	); \
	\
	out vec2 texCoords; \
	\
	void main() { \
		\
		gl_Position = vec4(verts[gl_VertexID] * 2.0f - 1.0f, 0.0f, 1.0f); \
		texCoords = verts[gl_VertexID]; \
		\
	}",

	"#version 430 core\n \
	in vec2 texCoords; \
	layout(location = 0) out vec4 fragColor; \
	\
	uniform sampler2DMS tex; \
	\
	void main() { \
		\
		vec4 color = vec4(0.0f); \
		ivec2 coords = ivec2(gl_FragCoord.xy); \
		for (int i = 0; i < 4; i++) color += texelFetch(tex, coords, i); \
		fragColor = color / 4.0f; \
		\
	}"

};

const char* GL::Scene::progLights_source[2] = {

	"#version 430 core\n \
	\
	const vec2 coords[6] = vec2[]( \
		vec2(0.0f, 0.0f), \
		vec2(1.0f, 0.0f), \
		vec2(1.0f, 1.0f), \
		vec2(1.0f, 1.0f), \
		vec2(0.0f, 1.0f), \
		vec2(0.0f, 0.0f) \
	); \
	\
	layout(std140, binding = 2) uniform raytraceUnis { \
		\
		uniform vec2 start; \
		uniform vec2 end; \
		uniform float sz; \
		uniform mat3 rot; \
		\
	}; \
	\
	out vec3 fragPos; \
	out vec2 screenCoords; \
	\
	void main() { \
		\
		gl_Position = vec4(coords[gl_VertexID] * 2.0f - 1.0f, -1.0f, 1.0f); \
		fragPos = rot * vec3(mix(start, end, coords[gl_VertexID]), sz); \
		screenCoords = coords[gl_VertexID]; \
		\
	}",

	"#version 430 core\n \
	\
	in vec3 fragPos; \
	in vec2 screenCoords; \
	\
	layout(std140, binding = 2) uniform raytraceUnis { \
		\
		uniform vec2 start; \
		uniform vec2 end; \
		uniform float sz; \
		uniform mat3 rot; \
		\
	}; \
	\
	uniform vec3 lightPositions[16]; \
	uniform vec3 lightColors[16]; \
	uniform float lightStrengths[16]; \
	uniform float lightRadaii[16]; \
	\
	uniform vec3 camPos; \
	uniform float zFar; \
	uniform sampler2DMS depthSampler; \
	\
	layout(location = 0) out vec4 fragColor; \
	\
	const uint numLights = 16u; \
	const float dropoff = 0.15f; \
	\
	void main() { \
		\
		float zNear = -sz; \
		ivec2 coords = ivec2(gl_FragCoord.xy); \
		float prevDepth = 0.0f; \
		for (int i = 0; i < 4; i++) prevDepth += texelFetch(depthSampler, coords, i).x; \
		prevDepth /= 4.0f; \
		prevDepth = 2.0f * prevDepth - 1.0f; \
		prevDepth = (2.0f * zNear * zFar) / (zNear + zFar - prevDepth * (zFar - zNear)); \
		prevDepth /= zFar; \
		prevDepth = zNear + prevDepth * (zFar - zNear); \
		\
		vec3 ray = fragPos / zNear; \
		vec4 totalColor = vec4(0.0f); \
		\
		for (uint i = 0u; i < numLights; i++) { \
			\
			if (lightStrengths[i] < 0.001f) continue; \
			\
			float depth = clamp(dot(ray, lightPositions[i] - camPos) / dot(ray, ray), 0.0f, prevDepth); \
			vec3 p = camPos + ray * depth; \
			vec3 diff = p - lightPositions[i]; \
			float dist = dot(diff, diff); \
			\
			float scale = lightStrengths[i] * 20.0f; \
			float lightStrength = scale / dist; \
			\
			totalColor += vec4(lightColors[i], 1.0f) * lightStrength; \
			\
		} \
		\
		fragColor = totalColor / (totalColor + vec4(1.0f)); \
		fragColor.rgb = pow(fragColor.rgb, vec3(1.0f / 2.2f)); \
		fragColor.a = max(0.0f, (dropoff + 1.0f) * fragColor.a - dropoff); \
		\
	}"

};

const char* GL::Scene::progSkybox_source[] = {

	"#version 430 core\n \
	\
	layout(std140, binding = 2) uniform raytraceUnis { \
		\
		uniform vec2 start; \
		uniform vec2 end; \
		uniform float sz; \
		uniform mat3 rot; \
		\
	}; \
	\
	const vec2 verts[6] = vec2[]( \
		vec2(0.0f, 0.0f), \
		vec2(1.0f, 0.0f), \
		vec2(1.0f, 1.0f), \
		vec2(1.0f, 1.0f), \
		vec2(0.0f, 1.0f), \
		vec2(0.0f, 0.0f) \
	); \
	\
	out vec3 ray; \
	\
	void main() { \
		\
		gl_Position = vec4(verts[gl_VertexID] * 2.0f - 1.0f, 0.0f, 1.0f); \
		ray = rot * vec3(mix(start, end, verts[gl_VertexID]), sz); \
		\
	}",

	"#version 430 core\n \
	\
	in vec3 ray; \
	layout(location = 0) out vec4 fragColor; \
	\
	uniform samplerCube skybox; \
	uniform float bgBrightness; \
	\
	void main() { \
		\
		vec3 sampleColor = texture(skybox, ray).rgb; \
		sampleColor *= bgBrightness; \
		sampleColor /= sampleColor + vec3(1.0f); \
		\
		fragColor = vec4(sampleColor, 1.0f); \
		\
	}"

};