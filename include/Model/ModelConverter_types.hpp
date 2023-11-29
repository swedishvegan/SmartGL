#ifndef	MODELCONVERTER_TYPES_HPP
#define MODELCONVERTER_TYPES_HPP

#include <string>
#include "./../util/GL-math.hpp"

namespace GL {
	
	namespace ModelConverter_types {

			struct VertexArrayData { 
			
			void* data; 
			unsigned int dataSize; 
			unsigned int vertexSize; 
			bool hasNormalMap; 
			VertexArrayData* next; 
			
		}; 

		enum class MaterialFormat { B, BMR, BN, BMRN }; 

		struct Material { 
			
			vec4 baseColor = vec4(); 
			std::string path_base; 
			std::string path_metallicRoughness; 
			std::string path_normal; 
			int baseIdx = -1; 
			int metallicRoughnessIdx = -1; 
			int normalIdx = -1; 
			unsigned char* baseTexData = nullptr; 
			unsigned char* metallicRoughnessTexData = nullptr; 
			unsigned char* normalTexData = nullptr; 
			unsigned int baseWidth = 0u; 
			unsigned int baseHeight = 0u; 
			unsigned int metallicRoughnessWidth = 0u; 
			unsigned int metallicRoughnessHeight = 0u; 
			unsigned int normalWidth = 0u; 
			unsigned int normalHeight = 0u; 
			
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
			
		}; 

		struct BoneNode { 
			
			std::string name; 
			unsigned int* childIndices = nullptr; 
			unsigned int numChildren; 
			unsigned int glIndex; 
			bool isBone; 
			
			Animation** animations = nullptr; 
			mat4 trans; 
			mat4 offset; 
			
			mat4 modelMatrix; 
			mat3 normalMatrix; 
			
		};

	}

}

#endif