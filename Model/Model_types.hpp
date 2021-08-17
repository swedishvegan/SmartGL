#ifndef MODEL_TYPES_HPP
#define MODEL_TYPES_HPP

#include "util/GL-math.hpp"

#define _GL_Model_numOptions_metallic 3
#define _GL_Model_numOptions_roughness 3
#define _GL_Model_numOptions_metallicRoughness (_GL_Model_numOptions_metallic * _GL_Model_numOptions_roughness + 1)
#define _GL_Model_numOptions_normal 3
#define _GL_Model_numOptions_albedo 3
#define _GL_Model_numOptions_skybox 2
#define _GL_Model_numOptions_animated 2

#define _GL_Model_programBase_metallic 1
#define _GL_Model_programBase_roughness (_GL_Model_programBase_metallic * _GL_Model_numOptions_metallic)
#define _GL_Model_programBase_metallicRoughness 1
#define _GL_Model_programBase_normal (_GL_Model_programBase_metallicRoughness * _GL_Model_numOptions_metallicRoughness)
#define _GL_Model_programBase_albedo (_GL_Model_programBase_normal * _GL_Model_numOptions_normal)
#define _GL_Model_programBase_skybox (_GL_Model_programBase_albedo * _GL_Model_numOptions_albedo)
#define _GL_Model_programBase_animated (_GL_Model_programBase_skybox * _GL_Model_numOptions_skybox)

#define _GL_Model_numVertShaders 4
#define _GL_Model_numFragShaders _GL_Model_programBase_animated
#define _GL_Model_numPrograms (_GL_Model_programBase_animated * _GL_Model_numOptions_animated)

#define _GL_Model_indexOffset_skybox 0
#define _GL_Model_indexOffset_albedo (_GL_Model_indexOffset_skybox + _GL_Model_numOptions_skybox)
#define _GL_Model_indexOffset_normal (_GL_Model_indexOffset_albedo + _GL_Model_numOptions_albedo)
#define _GL_Model_indexOffset_metallicRoughness (_GL_Model_indexOffset_normal + _GL_Model_numOptions_normal)
#define _GL_Model_indexOffset_usesTextures (_GL_Model_indexOffset_metallicRoughness + _GL_Model_numOptions_metallicRoughness)

#define _GL_Model_vertShaderVarCodeArrayLength 6
#define _GL_Model_fragShaderVarCodeArrayLength (_GL_Model_indexOffset_usesTextures + 2)

#define _GL_Model_defaultStretch 1.0f

#define _GL_Model_declareHelperTypes() \
struct VertexArrayData { \
	\
	void* data; \
	bool hasNormalMap; \
	VertexArrayData* next; \
	\
}; \
\
enum class MaterialFormat { B, BMR, BN, BMRN }; \
\
enum class SampleType { UNIFORM, TEXTURE_2D, TEXTURE_3D }; \
\
struct Material { \
	\
	GLuint baseTex = 0u; \
	GLuint metallicRoughnessTex = 0u; \
	GLuint normalTex = 0u; \
	vec4 baseColor = vec4(); \
	int baseIdx; \
	int metallicRoughnessIdx; \
	int normalIdx; \
	\
}; \
\
struct AnimationData { \
	\
	float duration; \
	float TPS; \
	\
}; \
 \
struct Animation { \
	\
	vec3* scalings; \
	float* scalingTimes; \
	unsigned int numScalings; \
	\
	vec4* rots; \
	float* rotTimes; \
	unsigned int numRots; \
	\
	vec3* translations; \
	float* translationTimes; \
	unsigned int numTranslations; \
	\
	static vec4 interpolateSpherical(vec4 a, vec4 b, float t) { \
		\
		float cosTheta = dot(a, b); \
		if (cosTheta >= 0.999f) return mix(a, b, t); \
		\
		vec4 diff = b - a; \
		float theta = std::acos(cosTheta); \
		vec2 pq = inverse(mat2( \
			dot(a, a), dot(b, a), \
			dot(a, diff), dot(b, diff) \
		)) * vec2(std::cos(theta * t), std::cos(theta * (1.0f - t))); \
		\
		return pq.x * a + pq.y * diff; \
		\
	} \
	\
	/* Code for this function copied from Assimp library */ \
	static mat4 quaternionToMatrix(vec4 q) { \
		\
		float a1 = 1.0f - 2.0f * (q.y * q.y + q.z * q.z); \
		float a2 = 2.0f * (q.x * q.y - q.z * q.w); \
		float a3 = 2.0f * (q.x * q.z + q.y * q.w); \
		float b1 = 2.0f * (q.x * q.y + q.z * q.w); \
		float b2 = 1.0f - 2.0f * (q.x * q.x + q.z * q.z); \
		float b3 = 2.0f * (q.y * q.z - q.x * q.w); \
		float c1 = 2.0f * (q.x * q.z - q.y * q.w); \
		float c2 = 2.0f * (q.y * q.z + q.x * q.w); \
		float c3 = 1.0f - 2.0f * (q.x * q.x + q.y * q.y); \
		\
		return mat4( \
			a1, b1, c1, 0.0f, \
			a2, b2, c2, 0.0f, \
			a3, b3, c3, 0.0f, \
			0.0f, 0.0f, 0.0f, 1.0f \
		); \
		\
	} \
	\
	static float normaliz(float t, float start, float end) { return (t - start) / (end - start); } \
	\
	static unsigned int findTimestampIndex(float t, float* timestamps, unsigned int numTimestamps) { \
		\
		unsigned int idx = 0u; \
		for (unsigned int i = 0u; i < numTimestamps; i++) if (timestamps[i] > t) { idx = i; break; } \
		return idx; \
		\
	} \
	\
	mat4 getMatrix(float t) { \
		\
		unsigned int transIdx = findTimestampIndex(t, translationTimes, numTranslations); \
		unsigned int rotIdx = findTimestampIndex(t, rotTimes, numRots); \
		unsigned int scaleIdx = findTimestampIndex(t, scalingTimes, numScalings); \
		\
		if (transIdx > 0u) transIdx--; \
		if (rotIdx > 0u) rotIdx--; \
		if (scaleIdx > 0u) scaleIdx--; \
		\
		float transT = normaliz(t, translationTimes[transIdx], translationTimes[transIdx + 1u]); \
		float rotT = normaliz(t, rotTimes[rotIdx], rotTimes[rotIdx + 1u]); \
		float scaleT = normaliz(t, scalingTimes[scaleIdx], scalingTimes[scaleIdx + 1u]); \
		\
		vec3 translation = mix(translations[transIdx], translations[transIdx + 1u], transT); \
		vec4 rot = interpolateSpherical(rots[rotIdx], rots[rotIdx + 1u], rotT); \
		vec3 scaling = mix(scalings[scaleIdx], scalings[scaleIdx + 1u], scaleT); \
		\
		return translate(translation) * quaternionToMatrix(rot) * scale(scaling); \
		\
	} \
	\
}; \
 \
struct BoneNode { \
	\
	unsigned int* childIndices = nullptr; \
	unsigned int numChildren; \
	unsigned int glIndex; \
	\
	Animation** animations = nullptr; \
	mat4 trans; \
	mat4 offset; \
	\
	mat4 modelMatrix; \
	mat3 normalMatrix; \
	\
}; \
\
struct ModelData { \
	\
	const char* name; \
	unsigned int referenceCount = 0u; \
	\
	unsigned int numMeshes = 0u; \
	\
	GLuint* vaos = nullptr; \
	GLuint* vaos_shadow; \
	GLuint* vbos; \
	GLuint* ebos; \
	\
	unsigned int numMats; \
	Material* mats = nullptr; \
	MaterialFormat* matFormats; \
	unsigned int* matIndices; \
	\
	unsigned int* numElements; \
	\
	BoneNode* boneNodes = nullptr; \
	unsigned int numBoneNodes = 0u; \
	unsigned int numBones = 0u; \
	unsigned int numAnimations = 0u; \
	\
	AnimationData* animationData = nullptr; \
	\
};

#endif