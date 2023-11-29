#ifndef VERTEXSTRUCTREPRESENTATION_HPP
#define VERTEXSTRUCTREPRESENTATION_HPP

#include "./../util/util.hpp"

namespace GL {

	struct VertexStructRepresentation { 
		
		enum ComponentType { POSITION, NORMAL, TEX_COORDS, TANGENT, BITANGENT, BONE_INDICES, BONE_WEIGHTS }; 
		
		VertexStructRepresentation();
		
		VertexStructRepresentation(bool usesTextures, bool usesNormalMap, bool usesAnimations);
		
		unsigned int getComponentOffset(ComponentType type); 
		
		bool hasComponent(ComponentType type);
		
		unsigned int dataSize = 0u; 
		
	private: 
		
		int numComponents = 0; 
		unsigned int componentOffsets[7]; 
		int componentIndices[7]; 
		
		static unsigned int getComponentSize(ComponentType type);
		
		void addComponent(ComponentType type); 
		
	};

}

#endif