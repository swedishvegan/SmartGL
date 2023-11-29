
#include "./VertexStructRepresentation.hpp"

GL::VertexStructRepresentation::VertexStructRepresentation() { } 

GL::VertexStructRepresentation::VertexStructRepresentation(bool usesTextures, bool usesNormalMap, bool usesAnimations) { 
    
    for (int i = 0; i < 7; i++) componentIndices[i] = -1; 
    
    addComponent(POSITION); 
    addComponent(NORMAL); 
    
    if (usesTextures) addComponent(TEX_COORDS); 
    
    if (usesNormalMap) { 
        addComponent(TANGENT); 
        addComponent(BITANGENT); 
    } 
    
    if (usesAnimations) { 
        addComponent(BONE_INDICES); 
        addComponent(BONE_WEIGHTS); 
    } 
    
} 

unsigned int GL::VertexStructRepresentation::getComponentOffset(ComponentType type) {  
    
    if (type > 6) throw Exception("Component type " + std::to_string((int)type) + " in VertexStructRepresentation::getComponentOffset() is too high; it must be in the range [0, 6]."); 
    if (componentIndices[type] == -1) throw Exception("Attempt to get the offset for the nonexistent VertexStructRepresentation component " + std::to_string((int)type) + "."); 
    return componentOffsets[componentIndices[type]]; 
    
} 

bool GL::VertexStructRepresentation::hasComponent(ComponentType type) { 
    
    if (type > 6) throw Exception("Component type " + std::to_string((int)type) + " in VertexStructRepresentation::hasComponent() is too high; it must be in the range [0, 6]."); 
    return componentIndices[type] >= 0; 
    
} 

unsigned int GL::VertexStructRepresentation::getComponentSize(ComponentType type) { 
    
    unsigned int numComps = type == TEX_COORDS ? 2u : (type == BONE_INDICES || type == BONE_WEIGHTS ? 4u : 3u); 
    return numComps * sizeof(float); 
    
} 

void GL::VertexStructRepresentation::addComponent(ComponentType type) { 
    
    componentOffsets[numComponents] = dataSize; 
    componentIndices[type] = numComponents; 
    numComponents++; 
    dataSize += getComponentSize(type); 
    
} 