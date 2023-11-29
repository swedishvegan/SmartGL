#ifndef MODEL_HPP
#define MODEL_HPP

#include <vector>

#include "./../Texture/Image.hpp"
#include "./../util/util.hpp"
#include "./../util/enums.hpp"
#include "./../util/BinaryFile.hpp"
#include "./../Program/Program.hpp"
#include "./../Uniform/UniformTable.hpp"
#include "./../Uniform/UniformBufferTable.hpp"
#include "./Scene.hpp"
#include "./../util/Timer.hpp"
#include "./Model_types.hpp"
#include "./ModelInstanceBuffer.hpp"
#include "./ModelProgram.hpp"

namespace GL {

	class Model : public _util, public Drawable {
	public:

		Model(const char* filePath, bool preCompilePrograms = false);

		void setModelMatrix(mat4 model);

		mat4 getModelMatrix() const;

		void setColor(vec3 color);

		void useDefaultColor();

		void setMetallicValue(float value);

		void setRoughnessValue(float value);

		void setAmbientOcclusion(float value);

		void setShadowOcclusion(float value);

		void setInstanceBuffer(ModelInstanceBuffer& instances);

		void useInstances(bool use);

		void setCustomProgram(ModelProgram& program);

		void useCustomProgram(bool use);

		float getMetallicValue() const;

		float getRoughnessValue() const;

		float getAmbientOcclusion() const;

		float getShadowOcclusion() const;

		unsigned int getNumAnimations() const;

		BoundingBox getBoundingBox() const;

		BoundingBox getWorldSpaceBoundingBoxApproximation() const;

		bool isAnimated() const;

		void playAnimation(unsigned int index, bool loop);

		void stopAnimation();

		void resetAnimation();

		bool isAnimationDone() const;

		bool isInstanced() const;

		void setSamplingFactor3D(TextureType type, float value);

		void draw(Scene& scene, SampleSettings reqSettings = SampleSettings{ });
		
		void drawShadow(mat4 PV, mat4 model, Scene& scene);

		~Model();

	protected:

		Timer timer;
		float t, t0;

		unsigned int animationIndex = 0u;
		bool isAnimationPlaying = false;
		bool isAnimationLooped;
		bool isAnimationFinished = false;

		mat4 model;
		ModelInstanceBuffer* instancesBuf = nullptr;
		bool shouldUseInstances = false;

		ModelProgram* customProg = nullptr;
		bool shouldUseProg = false;

		bool customColor = false;
		vec3 albedoColor;

		float metallic = 0.0f;
		float roughness = 1.0f;
		float ao = 1.0f;
		float shadowOcclusion = 0.45f;

		float albedoStretch = _GL_Model_defaultStretch;
		float normalStretch = _GL_Model_defaultStretch;
		float metallicStretch = _GL_Model_defaultStretch;
		float roughnessStretch = _GL_Model_defaultStretch;

		bool isPhysicsModel;
		
		unsigned int thisModelDataIndex;
		static std::vector<Model_types::ModelData> modelData;

		static ShaderLoader* PBR_vertShaders[_GL_Model_numVertShaders];
		static ShaderLoader* PBR_fragShaders[_GL_Model_numFragShaders];
		static Program* PBR_programs[_GL_Model_numPrograms];
		static UniformTable* PBR_uniforms[_GL_Model_numPrograms];
		static UniformBufferTable* PBR_commonUniforms;
		static UniformBufferTable* PBR_animationMatrices;
		static bool PBR_initialized;

		static const char* PBR_vert_variable_code[_GL_Model_vertShaderVarCodeArrayLength];
		static const char* PBR_frag_variable_code[_GL_Model_fragShaderVarCodeArrayLength];
		static const char* PBR_vert_base_code;
		static const char* PBR_frag_base_code;

		Model() { }

		void loadMaterials(ReadBinaryFile& rbf);

		void loadBoneNodes(ReadBinaryFile& rbf);

		virtual void loadMeshes(ReadBinaryFile& rbf);

		void updateUBOs(mat4 PV, mat4 modelMatrix, mat3 normalMatrix, Scene& scene, bool drawingShadow);

		void updateModelMatrices(unsigned int idx, mat4 globalTransform);

		int getProgramIndex(bool isAnimated, bool hasSkybox, Model_types::SampleType albedo, Model_types::SampleType normal, bool hasMetallicRoughnessTex, Model_types::SampleType metallic, Model_types::SampleType roughness);

		int getProgramIndex(unsigned int materialIndex, bool skybox);

		static void initUBOs();

		static void compileProgram(unsigned int idx);

	};

}

#define _GL_Model_constructor(bboxDeclaration) \
thisModelDataIndex = modelData.size(); \
for (unsigned int i = 0u; i < modelData.size(); i++) \
\
if (strcmp(modelData[i].name, filePath) == 0) { \
	\
	thisModelDataIndex = i; \
	break; \
	\
} \
\
if (thisModelDataIndex == modelData.size()) { \
	\
	modelData.push_back(Model_types::ModelData{ }); \
	modelData[thisModelDataIndex].name = filePath; \
	\
	ReadBinaryFile rbf(filePath, 1024 * 512); \
	\
	static char verify[5]; for (int i = 0; i < 5; i++) verify[i] = rbf.readByte(); \
	if (verify[0] != 'm' || verify[1] != 'o' || verify[2] != 'd' || verify[3] != 'e' || verify[4] != 'l') throw Exception("Model file \"" + std::string(filePath) + "\" verification failed."); \
	\
	loadMaterials(rbf); \
	loadBoneNodes(rbf); \
	bboxDeclaration \
	loadMeshes(rbf); \
	\
	if (!PBR_initialized) { \
		\
		for (int i = 0; i < _GL_Model_numVertShaders; i++) PBR_vertShaders[i] = nullptr; \
		for (int i = 0; i < _GL_Model_numFragShaders; i++) PBR_fragShaders[i] = nullptr; \
		for (int i = 0; i < _GL_Model_numPrograms; i++) { PBR_programs[i] = nullptr; PBR_uniforms[i] = nullptr; } \
		PBR_initialized = true; \
		\
	} \
	\
	if (preCompilePrograms) for (unsigned int i = 0u; i < modelData[thisModelDataIndex].numMeshes; i++) { \
		\
		compileProgram((unsigned int)getProgramIndex((unsigned int)modelData[thisModelDataIndex].matIndices[i], false)); \
		compileProgram((unsigned int)getProgramIndex((unsigned int)modelData[thisModelDataIndex].matIndices[i], true)); \
		\
	} \
	\
} \
\
modelData[thisModelDataIndex].referenceCount++;

#define _GL_Model_loadMeshes(physicsCode0, physicsCode1, physicsCode2) \
modelData[thisModelDataIndex].numMeshes = rbf.read<unsigned int>(); \
if (modelData[thisModelDataIndex].numMeshes) { \
	\
	physicsCode0 \
	\
	modelData[thisModelDataIndex].vaos = new GLuint[modelData[thisModelDataIndex].numMeshes]; \
	modelData[thisModelDataIndex].vaos_shadow = new GLuint[modelData[thisModelDataIndex].numMeshes]; \
	modelData[thisModelDataIndex].vbos = new GLuint[modelData[thisModelDataIndex].numMeshes]; \
	modelData[thisModelDataIndex].ebos = new GLuint[modelData[thisModelDataIndex].numMeshes]; \
	modelData[thisModelDataIndex].numElements = new unsigned int[modelData[thisModelDataIndex].numMeshes]; \
	modelData[thisModelDataIndex].matIndices = new unsigned int[modelData[thisModelDataIndex].numMeshes]; \
	\
	for (unsigned int i = 0u; i < modelData[thisModelDataIndex].numMeshes; i++) { \
		\
		unsigned int dataSize = rbf.read<unsigned int>(); \
		unsigned int vertexSize = rbf.read<unsigned int>(); \
		bool hasNormalMap = rbf.read<bool>(); \
		modelData[thisModelDataIndex].numElements[i] = rbf.read<unsigned int>(); \
		modelData[thisModelDataIndex].matIndices[i] = rbf.read<unsigned int>(); \
		\
		if (dataSize && modelData[thisModelDataIndex].numElements[i]) { \
			\
			char* vertexData = new char[dataSize]; \
			for (unsigned int j = 0u; j < dataSize; j++) vertexData[j] = rbf.readByte(); \
			\
			unsigned int* indexData = new unsigned int[modelData[thisModelDataIndex].numElements[i]]; \
			for (unsigned int j = 0u; j < modelData[thisModelDataIndex].numElements[i]; j++) indexData[j] = rbf.read<unsigned int>(); \
			bool deleteIndices = true; \
			\
			glGenVertexArrays(1, &modelData[thisModelDataIndex].vaos[i]); \
			glBindVertexArray(modelData[thisModelDataIndex].vaos[i]); \
			\
			glGenBuffers(1, &modelData[thisModelDataIndex].vbos[i]); \
			glBindBuffer(GL_ARRAY_BUFFER, modelData[thisModelDataIndex].vbos[i]); \
			\
			glGenBuffers(1, &modelData[thisModelDataIndex].ebos[i]); \
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelData[thisModelDataIndex].ebos[i]); \
			\
			glBufferData(GL_ARRAY_BUFFER, dataSize, vertexData, GL_STATIC_DRAW); \
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * modelData[thisModelDataIndex].numElements[i], indexData, GL_STATIC_DRAW); \
			\
			unsigned int offset = 0u; \
			Model_types::Material& m = modelData[thisModelDataIndex].mats[modelData[thisModelDataIndex].matIndices[i]]; \
			bool hasTextures = m.baseTex > 0u || m.normalTex > 0u || m.metallicRoughnessTex > 0u; \
			\
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertexSize, (void*)(unsigned long long)offset); offset += sizeof(vec3); \
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vertexSize, (void*)(unsigned long long)offset); offset += sizeof(vec3); \
			glEnableVertexAttribArray(0); \
			glEnableVertexAttribArray(1); \
			\
			if (hasTextures) { \
				\
				glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, vertexSize, (void*)(unsigned long long)offset); offset += sizeof(vec2); \
				glEnableVertexAttribArray(2); \
				\
			} \
			\
			if (hasNormalMap) { \
				\
				glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, vertexSize, (void*)(unsigned long long)offset); offset += sizeof(vec3); \
				glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, vertexSize, (void*)(unsigned long long)offset); offset += sizeof(vec3); \
				glEnableVertexAttribArray(3); \
				glEnableVertexAttribArray(4); \
				\
			} \
			\
			unsigned int boneDataOffset = offset; \
			\
			if (modelData[thisModelDataIndex].boneNodes) { \
				\
				glVertexAttribIPointer(5, 4, GL_INT, vertexSize, (void*)(unsigned long long)offset); offset += sizeof(ivec4); \
				glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, vertexSize, (void*)(unsigned long long)offset); \
				glEnableVertexAttribArray(5); \
				glEnableVertexAttribArray(6); \
				\
			} \
			\
			glGenVertexArrays(1, &modelData[thisModelDataIndex].vaos_shadow[i]); \
			glBindVertexArray(modelData[thisModelDataIndex].vaos_shadow[i]); \
			\
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelData[thisModelDataIndex].ebos[i]); \
			\
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertexSize, (void*)0); \
			glEnableVertexAttribArray(0); \
			\
			if (modelData[thisModelDataIndex].boneNodes) { \
				\
				glVertexAttribIPointer(5, 4, GL_INT, vertexSize, (void*)(unsigned long long)boneDataOffset); boneDataOffset += sizeof(ivec4); \
				glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, vertexSize, (void*)(unsigned long long)boneDataOffset); \
				glEnableVertexAttribArray(5); \
				glEnableVertexAttribArray(6); \
				\
			} \
			\
			physicsCode1 \
			\
			delete[] vertexData; \
			if (deleteIndices) delete[] indexData; \
			\
		} \
		else modelData[thisModelDataIndex].vaos[i] = 0u; \
		\
	} \
	\
	glBindVertexArray(0); \
	\
	physicsCode2 \
	\
}

#endif