
#include "./ModelProgram.hpp"

GL::ModelShader::ModelShader(GL::ShaderType shaderType) : sType(shaderType) { if (!(sType == ShaderType::VERTEX || sType == ShaderType::FRAGMENT)) throw Exception("A model shader must either be a vertex or fragment shader."); }

bool GL::ModelShader::isInitialized() const { return isInit; }

GL::ShaderLoader& GL::ModelShader::getShader(unsigned int idx) {

	if (!isInit) throw Exception("Attempt to get shader from uninitialized ModelShader.");
	
	idx %= _GL_ModelProgram_numPrograms;
	compileShader(idx);
	return *loaders[idx];

}

GL::ShaderType GL::ModelShader::getType() const { return sType; }

GL::ModelShader::~ModelShader() {
	
	if (loaders) {
		
		if (isInit) for (int i = 0; i < _GL_ModelProgram_numPrograms; i++) delete loaders[i];
		delete[] loaders;

	}

}

void GL::ModelShader::compileShader(unsigned int idx) {

	if (!loaders) {

		loaders = new ShaderLoader*[_GL_ModelProgram_numPrograms];
		for (int i = 0; i < _GL_ModelProgram_numPrograms; i++) loaders[i] = nullptr;

	}
	if (loaders[idx]) return;

	const char* source = (sType == ShaderType::VERTEX) ? vs_source : fs_source;

#define _GL_ModelShader_extractCoefficient(name) \
int name = j / _GL_ModelProgram_programBase_ ## name; j %= _GL_ModelProgram_programBase_ ## name;

	int j = idx;
	_GL_ModelShader_extractCoefficient(hasShadowMap)
		_GL_ModelShader_extractCoefficient(hasMetallicRoughnessMap)
		_GL_ModelShader_extractCoefficient(hasAlbedoMap)
		_GL_ModelShader_extractCoefficient(hasNormalMap)
		_GL_ModelShader_extractCoefficient(isAnimated)
		int isTextured = hasAlbedoMap | hasNormalMap | hasMetallicRoughnessMap;

	loaders[idx] = new ShaderLoader(sType);
	try {

		loaders[idx]->init(
			"#version 430 core\n", false,
			macroHeaders[isTextured], false,
			macroHeaders[2 + isAnimated], false,
			macroHeaders[4 + hasNormalMap], false,
			macroHeaders[6 + hasAlbedoMap], false,
			macroHeaders[8 + hasMetallicRoughnessMap], false,
			macroHeaders[10 + hasShadowMap], false,
			source, false,
			commonCode.getCodeString().c_str(), false
		);

	}
	catch (Exception& e) { throw e; }

}

GL::ModelProgram::ModelProgram(GL::ModelShader& vertexShader, GL::ModelShader& fragmentShader) : vs(&vertexShader), fs(&fragmentShader) {
	
	if (vertexShader.getType() != ShaderType::VERTEX) throw Exception("First shader passed to ModelProgram must be a vertex shader.");
	if (fragmentShader.getType() != ShaderType::FRAGMENT) throw Exception("Second shader passed to ModelProgram must be a fragment shader.");
	
}

GL::Program& GL::ModelProgram::getProgram(GL::ModelFormat format) { 
	
	linkProgram(getIndex(format));
	return *programs[getIndex(format)];
	
}

GL::UniformTable& GL::ModelProgram::getUniformTable() { 
	
	initializeUniforms();
	return *unis;

}

void GL::ModelProgram::prepareForUse(GL::ModelFormat format) {

	if (!customUnis) return;

	unsigned int curUnit = 0u;
	unsigned int maxUnit = _util::maxTextureUnits - 1;
	unsigned int progIdx = getIndex(format);

	for (unsigned int i = 0u; i < numTextures; i++) {

		if (curUnit < 16u) while (occupiedUnits[curUnit]) {

			curUnit++;
			if (curUnit == 16u) break;

		}

		if (curUnit == maxUnit) throw Exception("Too many textures were passed to ModelProgram. OpenGL ran out of available texture units (maximum allowed number is " + std::to_string(_util::maxTextureUnits) + ").");
		
		customUnis->set<int>(samplers[i], curUnit);
		textures[i]->bindToTextureUnit(curUnit);
		
		curUnit++;

	}

	linkProgram(progIdx);
	customUnis->update(*programs[progIdx]);

}

void GL::ModelProgram::markUnitAsOccupied(unsigned int unit, bool occupied) { occupiedUnits[unit % 16u] = occupied; }

GL::ModelProgram::~ModelProgram() {

	if (programs) {

		for (int i = 0; i < _GL_ModelProgram_numPrograms; i++) if (programs[i]) delete programs[i];
		delete[] programs;

		if (unis) delete unis;

	}

	if (customUnis) delete customUnis;

}

void GL::ModelProgram::linkProgram(unsigned int idx) {

	if (!programs) {

		programs = new Program*[_GL_ModelProgram_numPrograms];
		for (int i = 0; i < _GL_ModelProgram_numPrograms; i++) programs[i] = nullptr;

	}
	if (programs[idx]) return;

	unsigned int hasShadowMap = idx / _GL_ModelProgram_programBase_hasShadowMap;

	programs[idx] = new Program();
	try { programs[idx]->init(vs->getShader(idx), fs->getShader(idx)); }
	catch (Exception& e) { throw e; }

}

void GL::ModelProgram::initializeUniforms() {

	if (unis) return;

#define _GL_ModelProgram_unis \
"isInstanced", UniformType::INT, 1, \
"_albedoSampler", UniformType::INT, 1, \
"_normalSampler", UniformType::INT, 1, \
"_metallicRoughnessSampler", UniformType::INT, 1, \
"_color", UniformType::VEC4, 1, \
"metallic", UniformType::FLOAT, 1, \
"roughness", UniformType::FLOAT, 1, \
"_shadowSamplers", UniformType::INT, 6, \
"_hasShadowSampler", UniformType::INT, 6, \
"_overheadShadowSampler", UniformType::INT, 2, \
"_overheadPVMatrix", UniformType::MAT4, 2, \
"_overheadFarPlane", UniformType::FLOAT, 2, \
"_bboxStart", UniformType::VEC3, 2, \
"_bboxEnd", UniformType::VEC3, 2, \
"overheadLightDirection", UniformType::VEC3, 1, \
"overheadLightColor", UniformType::VEC3, 1

	unis = new UniformTable();
	unis->init(_GL_ModelProgram_unis);

	unis->set("_albedoSampler", ALBEDO_TEXTURE_UNIT);
	unis->set("_normalSampler", NORMAL_TEXTURE_UNIT);
	unis->set("_metallicRoughnessSampler", METALLIC_ROUGHNESS_TEXTURE_UNIT);
	for (int j = 0; j < 6; j++) unis->setElement("_shadowSamplers", j, 8 + j);
	for (int j = 0; j < 2; j++) unis->setElement("_overheadShadowSampler", j, 14 + j);

}

void GL::ModelProgram::bindTextures_impl(GL::TextureBase& texture, const char* samplerName) {

	unsigned int uc = customUnis->getUniformCount(samplerName, UniformType::INT);
	if (uc != 1u) throw Exception("Texture sampler name \"" + std::string(samplerName) + "\" was not declared as a sampler when ModelProgram's uniforms were initialized.");
	
	if (numTextures == textures.size()) {

		textures.push_back(&texture);
		samplers.push_back(samplerName);

	}
	else {

		textures[numTextures] = &texture;
		samplers[numTextures] = samplerName;

	}
	numTextures++;

}

unsigned int GL::ModelProgram::getIndex(ModelFormat format) {

	return (
		format.hasAlbedoMap * _GL_ModelProgram_programBase_hasAlbedoMap +
		format.hasMetallicRoughnessMap * _GL_ModelProgram_programBase_hasMetallicRoughnessMap +
		format.hasNormalMap * _GL_ModelProgram_programBase_hasNormalMap +
		format.hasShadowMap * _GL_ModelProgram_programBase_hasShadowMap +
		format.isAnimated * _GL_ModelProgram_programBase_isAnimated
	);

}

const char* GL::ModelShader::macroHeaders[2 * _GL_ModelProgram_numMacroOptions] = {

	" ",
	"\n#define USES_TEXTURES\n",

	" ",
	"\n#define ANIMATED\n",

	" ",
	"\n#define NORMAL_2D_TEXTURE\n",

	" ",
	"\n#define ALBEDO_2D_TEXTURE\n",

	" ",
	"\n#define METALLIC_ROUGHNESS_SAMPLER\n",

	" ",
	"\n#define SHADOW_MAP\n"

};

const char* GL::ModelShader::vs_source = \
"layout(location = 0) in vec3 localPosition; \
layout(location = 1) in vec3 normal; \
\
\n#ifdef USES_TEXTURES\n \
layout(location = 2) in vec2 textureCoordinates; \
out vec2 out_textureCoordinates; \
\n#endif\n \
\
\n#ifdef NORMAL_2D_TEXTURE\n \
layout(location = 3) in vec3 tangent; \
layout(location = 4) in vec3 bitangent; \
\n#endif\n \
\
\n#ifdef ANIMATED\n \
layout(location = 5) in ivec4 boneIndices; \
layout(location = 6) in vec4 boneWeights; \
\n#endif\n \
\
layout(std430, binding = 0) buffer _modelMatrixInstances { mat4 _modelMatrices[]; }; \
layout(std430, binding = 1) buffer _normalMatrixInstances { mat3 _normalMatrices[]; }; \
\
layout(std140, binding = 0) uniform PBR_inputs { \
	\
	mat4 _modelMat; \
	mat3 _normalMat; \
	vec3 lightPositions[16]; \
	vec3 lightColors[16]; \
	mat4 PVMatrix; \
	vec3 cameraPosition; \
	\
}; \
\
uniform int isInstanced; \
const mat4 _modelMatrix_temp = (isInstanced == 1) ? _modelMatrices[gl_InstanceID] : _modelMat; \
const mat3 _normalMatrix_temp = (isInstanced == 1) ? _normalMatrices[gl_InstanceID] : _normalMat; \
\
\n#ifdef ANIMATED\n \
layout(std140, binding = 1) uniform boneData { \
	\
	mat4 _boneModelMatrices[64]; \
	mat3 _boneNormalMatrices[64]; \
	\
}; \
\n#endif\n \
\
out vec3 out_position; \
\
\n#ifdef NORMAL_2D_TEXTURE\n \
const mat3 TBNMatrix = mat3(tangent, bitangent, normal); \
out mat3 out_TBNMatrix; \
\n#else\n \
out vec3 out_normal; \
\n#endif\n \
\
mat4 _getModelMatrix() { \
	\
	\n#ifdef ANIMATED\n \
	mat4 boneModelMatrix = mat4(0.0f); \
	for (uint i = 0u; i < 4u; i++) { boneModelMatrix += _boneModelMatrices[boneIndices[i]] * boneWeights[i]; } \
	mat4 finalModelMatrix = _modelMatrix_temp * boneModelMatrix; \
	\n#else\n \
	mat4 finalModelMatrix = _modelMatrix_temp; \
	\n#endif\n \
	\
	return finalModelMatrix; \
	\
} \
\
mat3 _getNormalMatrix() { \
	\
	\n#ifdef ANIMATED\n \
	mat3 boneNormalMatrix = mat3(0.0f); \
	for (uint i = 0u; i < 4u; i++) { boneNormalMatrix += _boneNormalMatrices[boneIndices[i]] * boneWeights[i]; } \
	mat3 finalNormalMatrix = _normalMatrix_temp * boneNormalMatrix; \
	\n#else\n \
	mat3 finalNormalMatrix = _normalMatrix_temp; \
	\n#endif\n \
	\
	return finalNormalMatrix; \
	\
} \
\
const mat4 modelMatrix = _getModelMatrix(); \
const mat3 normalMatrix = _getNormalMatrix(); \
\
vec3 position = (modelMatrix * vec4(localPosition, 1.0f)).xyz; \
\
void GL_Main(); \
\
void main() { \
	\
	GL_Main(); \
	\
	\n#ifdef NORMAL_2D_TEXTURE\n \
	out_TBNMatrix = normalMatrix * mat3(tangent, bitangent, normal); \
	\n#else\n \
	out_normal = normalize(normalMatrix * normal); \
	\n#endif\n \
	\
	\n#ifdef USES_TEXTURES\n \
	out_textureCoordinates = textureCoordinates; \
	\n#endif\n \
	\
	out_position = position; \
	gl_Position = PVMatrix * vec4(position, 1.0f); \
	\
}";

const char* GL::ModelShader::fs_source = \
"in vec3 out_position; \
\n#define position out_position\n \
\
\n#ifdef USES_TEXTURES\n \
in vec2 out_textureCoordinates; \
\n#define textureCoordinates out_textureCoordinates\n \
\n#endif\n \
\
layout(std140, binding = 0) uniform PBR_inputs { \
	\
	mat4 _modelMat; \
	mat3 _normalMat; \
	vec3 lightPositions[16]; \
	vec3 lightColors[16]; \
	mat4 PVMatrix; \
	vec3 cameraPosition; \
	\
}; \
\
\n#ifdef ALBEDO_2D_TEXTURE\n \
uniform sampler2D _albedoSampler; \
const vec4 _color = texture(_albedoSampler, out_textureCoordinates); \
\n#else\n \
uniform vec4 _color; \
\n#endif\n \
const vec3 color = _color.rgb; \
\
\n#ifdef NORMAL_2D_TEXTURE\n \
in mat3 out_TBNMatrix; \
uniform sampler2D _normalSampler; \
const vec3 normal = normalize(out_TBNMatrix * (2.0f * texture(_normalSampler, out_textureCoordinates).xyz - 1.0f)); \
\n#else\n \
in vec3 out_normal; \
const vec3 normal = normalize(out_normal); \
\n#endif\n \
\
\n#ifdef METALLIC_ROUGHNESS_SAMPLER\n \
uniform sampler2D _metallicRoughnessSampler; \
const vec2 _metallicRoughness = texture(_metallicRoughnessSampler, out_textureCoordinates).rg; \
const float metallic = _metallicRoughness.r; \
const float roughness = _metallicRoughness.g; \
\n#else\n \
uniform float metallic; \
uniform float roughness; \
\n#endif\n \
\
layout(location = 0) out vec4 _out_color; \
\
\n#ifdef SHADOW_MAP\n \
uniform samplerCube _shadowSamplers[6]; \
uniform int _hasShadowSampler[6]; \
\
uniform sampler2D _overheadShadowSampler[2]; \
uniform mat4 _overheadPVMatrix[2]; \
uniform float _overheadFarPlane[2]; \
uniform vec3 _bboxStart[2]; \
uniform vec3 _bboxEnd[2]; \
uniform vec3 overheadLightDirection; \
uniform vec3 overheadLightColor; \
\
const float _epsilon = 0.0002f; \
const float _shadowDecay = 0.3f; \
const float _offsetDist = 0.0013f; \
const int _numSamples = 20; \
const float _overheadSampleDist = 0.0004f; \
const float _bufferValue = 1.001f; \
const float _numOverheadSamples = 6.0f; \
const float _overheadIncrement = 2.0f * _overheadSampleDist / (_numOverheadSamples - 1.0f); \
const float _overheadShadowDecay = 4.0f; \
const float _overheadCutoff = 0.6f; \
const float _lightThreshold = 0.001f; \
\
const vec3 _offsets[_numSamples] = vec3[]( \
	vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1), \
	vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1), \
	vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0), \
	vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1), \
	vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1) \
); \
\
float getShadowValue(uint i) { \
	\
	if (i >= 6u) return 1.0f; \
	if (_hasShadowSampler[i] == 0) return 1.0f; \
	\
	vec3 sampleVector = position - lightPositions[i]; \
	float dist = length(sampleVector); \
	float NdotL = clamp(dot(normal, -sampleVector), 0.0f, 1.0f); \
	\
	float bias = clamp(0.005f * tan(acos(NdotL)), 0.0f, 0.01f); \
	\
	float totalShadowValue = 0.0f; \
	float minShadow, maxShadow; \
	\
	for (int i = 0; i < _numSamples; i++) { \
		\
		float shadowMapDist = texture(_shadowSamplers[i], sampleVector + _offsets[i] * _offsetDist).r + _epsilon * dist * bias; \
		float shadowValue = 1.0f - max(dist - shadowMapDist, 0.0f); \
		shadowValue = clamp(shadowValue * _shadowDecay, 0.0f, 1.0f); \
		\
		totalShadowValue += shadowValue; \
		\
	} \
	\
	return totalShadowValue / float(_numSamples); \
	\
} \
\
float _getOverheadShadowValue_atIndex(float NdotL, uint i) { \
	\
	if (_overheadFarPlane[i] > 0.0f) { \
		\
		float bias = 0.25f * clamp(0.005f * tan(acos(NdotL)), 0.0f, 0.01f); \
		\
		float totalShadowValue = 0.0f; \
		float totalNumSamples = 0.0f; \
		\
		for (float x = -_overheadSampleDist; x <= _overheadSampleDist * _bufferValue; x += _overheadIncrement) \
			for (float y = -_overheadSampleDist; y <= _overheadSampleDist * _bufferValue; y += _overheadIncrement) { \
				\
				vec3 shadowNDC = clamp((_overheadPVMatrix[i] * vec4(out_position, 1.0f)).xyz * 0.5f + 0.5f, 0.0f, 1.0f); \
				float dist = _overheadFarPlane[i] * shadowNDC.z; \
				float shadowMapDist = _overheadFarPlane[i] * texture(_overheadShadowSampler[i], shadowNDC.xy + vec2(x, y)).x + _epsilon * dist * bias; \
				float shadowValue = 1.0f - max(dist - shadowMapDist, 0.0f); \
				shadowValue = clamp(shadowValue * _overheadShadowDecay, 0.0f, 1.0f); \
				\
				totalShadowValue += shadowValue; \
				totalNumSamples += 1.0f; \
				\
		} \
		\
		return min(totalShadowValue / (float(totalNumSamples) * _overheadCutoff), 1.0f); \
		\
	} \
	else return 1.0f; \
	\
} \
\
float getOverheadShadowValue() { \
	\
	float NdotL = clamp(dot(normal, -overheadLightDirection), 0.0f, 1.0f); \
	\
	if (_overheadFarPlane[1] == 0.0f) return _getOverheadShadowValue_atIndex(NdotL, 0u); \
	\
	vec3 t = (out_position - _bboxStart[0]) / (_bboxEnd[0] - _bboxStart[0]); \
	t = 20.0f * min(t, 1.0f - t); \
	t = 1.0f - clamp(t, 0.0f, 1.0f); \
	float ts = max(t.x, max(t.y, t.z)); \
	\
	if (ts == 0.0f) return _getOverheadShadowValue_atIndex(NdotL, 0u); \
	else if (ts == 1.0f) return _getOverheadShadowValue_atIndex(NdotL, 1u); \
	else return mix(_getOverheadShadowValue_atIndex(NdotL, 0u), _getOverheadShadowValue_atIndex(NdotL, 1u), ts); \
	\
} \
\n#else\n \
float getShadowValue(uint i) { return 1.0f; } \
float getOverheadShadowValue() { return 1.0f; } \
\n#endif\n \
\
vec3 out_color; \
\
void GL_Main(); \
\
void main() { \
	\
	GL_Main(); \
	_out_color = vec4(out_color, 1.0f); \
	\
 }";
 