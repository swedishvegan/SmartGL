#ifndef MODEL_TYPES_HPP
#define MODEL_TYPES_HPP

#include "./../util/GL-math.hpp"
#include "./../util/util.hpp"
#include "./ModelStructs.hpp"

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


namespace GL {

	namespace Model_types {

		struct VertexArrayData { 
		
			void* data; 
			bool hasNormalMap; 
			VertexArrayData* next; 
			
		}; 

		enum class MaterialFormat { B, BMR, BN, BMRN }; 

		enum class SampleType { UNIFORM, TEXTURE_2D, TEXTURE_3D }; 

		struct Material { 
			
			GLuint baseTex = 0u; 
			GLuint metallicRoughnessTex = 0u; 
			GLuint normalTex = 0u; 
			vec4 baseColor = vec4(); 
			int baseIdx; 
			int metallicRoughnessIdx; 
			int normalIdx; 
			
		}; 

		struct AnimationData { 
			
			float duration; 
			float TPS; 
			
		}; 
		
		struct Animation { 
			
			vec3* scalings; 
			float* scalingTimes; 
			unsigned int numScalings; 
			
			vec4* rots; 
			float* rotTimes; 
			unsigned int numRots; 
			
			vec3* translations; 
			float* translationTimes; 
			unsigned int numTranslations; 
			
			static vec4 interpolateSpherical(vec4 a, vec4 b, float t);
			
			/* Code for this function copied from Assimp library */ 
			static mat4 quaternionToMatrix(vec4 q); 
			
			static float normaliz(float t, float start, float end);
			
			static unsigned int findTimestampIndex(float t, float* timestamps, unsigned int numTimestamps);
			
			mat4 getMatrix(float t);
			
		}; 
		
		struct BoneNode { 
			
			unsigned int* childIndices = nullptr; 
			unsigned int numChildren; 
			unsigned int glIndex; 
			
			Animation** animations = nullptr; 
			mat4 trans; 
			mat4 offset; 
			
			mat4 modelMatrix; 
			mat3 normalMatrix; 
			
		}; 

		struct ModelData { 
			
			const char* name; 
			unsigned int referenceCount = 0u; 
			
			BoundingBox bbox; 
			
			unsigned int numMeshes = 0u; 
			
			GLuint* vaos = nullptr; 
			GLuint* vaos_shadow; 
			GLuint* vbos; 
			GLuint* ebos; 
			
			unsigned int numMats; 
			Material* mats = nullptr; 
			MaterialFormat* matFormats; 
			unsigned int* matIndices; 
			
			unsigned int* numElements; 
			
			BoneNode* boneNodes = nullptr; 
			unsigned int numBoneNodes = 0u; 
			unsigned int numBones = 0u; 
			unsigned int numAnimations = 0u; 
			
			AnimationData* animationData = nullptr; 
			
		};

	}

}

#endif