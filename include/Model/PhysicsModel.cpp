
#include "./PhysicsModel.hpp"

#ifndef SmartGL_NO_PHYSICS

std::vector<GL::PhysicsModel::PhysicsModelData> GL::PhysicsModel::physicsModelData;

GL::PhysicsObject& GL::PhysicsModel::getPhysicsObject() const { return *po; }

GL::mat4 GL::PhysicsModel::getModelMatrix() const { return GL::translate(po->getPosition() - center) * GL::upscale<btMat4>(po->getRotation()) * GL::scale(btVec3(physicsModelData[thisPhysicsModelDataIndex].scale)); }

GL::PhysicsModel::~PhysicsModel() {

	if (po) delete po;

	physicsModelData[thisPhysicsModelDataIndex].referenceCount--;
	if (physicsModelData[thisPhysicsModelDataIndex].referenceCount == 0u) {
		
		delete physicsModelData[thisPhysicsModelDataIndex].shape;

		if (!physicsModelData[thisPhysicsModelDataIndex].bulletVertices) return;
			
		for (unsigned int i = 0u; i < modelData[thisModelDataIndex].numMeshes; i++) {
					
			delete[] physicsModelData[thisPhysicsModelDataIndex].bulletVertices[i];
			delete[](unsigned int*)physicsModelData[thisPhysicsModelDataIndex].bulletIndices[i];
					
		} 
				
		delete[] physicsModelData[thisPhysicsModelDataIndex].bulletVertices;
		delete[] physicsModelData[thisPhysicsModelDataIndex].bulletIndices;

	}

}

GL::PhysicsModel::PhysicsModel(const char* name, int type, btScalar scale) {
	
	thisPhysicsModelDataIndex = physicsModelData.size();
	shouldConstructNewShape = true;

	for (unsigned int i = 0u; i < physicsModelData.size(); i++)

		if (strcmp(name, physicsModelData[i].name) == 0) {
			
			if (type != physicsModelData[i].type) throw Exception("The engine does not allow loading two different physics models with the same source file ('" + std::string(name) + "') but different shapes.");
			if (scale != physicsModelData[i].scale) continue;

			thisPhysicsModelDataIndex = i;
			shouldConstructNewShape = false;
			break;

		}

	if (shouldConstructNewShape) {

		physicsModelData.push_back(PhysicsModelData{ });
		physicsModelData[thisPhysicsModelDataIndex].name = name;
		physicsModelData[thisPhysicsModelDataIndex].type = type;
		physicsModelData[thisPhysicsModelDataIndex].scale = scale;

	}

	physicsModelData[thisPhysicsModelDataIndex].referenceCount++;

}

#define _MakePhysicsModelClassImplementation(shapeType, typeCalculation, centerCalculation, physicsCode0, physicsCode1, physicsCode2) \
GL::shapeType ## PhysicsModel::shapeType ## PhysicsModel(const char* filePath, const PhysicsWorld& world, btScalar scale, btScalar mass, bool isKinematic, btVec3 offset, btMat3 rotation, bool preCompilePrograms) : PhysicsModel(filePath, typeCalculation, scale) { \
	\
	isPhysicsModel = true; \
	this->mass = mass; \
	this->isKinematic = isKinematic; \
	\
	_GL_Model_constructor(bbox = modelData[thisModelDataIndex].bbox;); \
	\
	center = centerCalculation; \
	po = new PhysicsObject(*physicsModelData[thisPhysicsModelDataIndex].shape, world, mass, isKinematic, offset + center, rotation); \
	\
} \
\
void GL::shapeType ## PhysicsModel::loadMeshes(ReadBinaryFile& rbf) { \
	\
	if (shouldConstructNewShape) { _GL_Model_loadMeshes(physicsCode0, physicsCode1, physicsCode2); } \
	else { _GL_Model_loadMeshes(,,); } \
	\
}

#define _GL_PhysicsModel_makeBoundingBox() \
bbox.start *= (float)physicsModelData[thisPhysicsModelDataIndex].scale; bbox.end *= (float)physicsModelData[thisPhysicsModelDataIndex].scale; \
btVector3 halfSideLengths = toBulletVector(abs(bbox.start - bbox.end) / 2.0f); \
btBoxShape* btBbox = new btBoxShape(halfSideLengths); \
physicsModelData[thisPhysicsModelDataIndex].shape = new PhysicsShape(btBbox);

_MakePhysicsModelClassImplementation(BoundingBox, 0, (bbox.start + bbox.end) / 2.0f, _GL_PhysicsModel_makeBoundingBox(),,);

#define _GL_PhysicsModel_makeBoundingSphere() \
bbox.start *= (float)physicsModelData[thisPhysicsModelDataIndex].scale; bbox.end *= (float)physicsModelData[thisPhysicsModelDataIndex].scale; \
btVec3 halfSideLengths = toBulletVector(abs(bbox.start - bbox.end) / 2.0f); \
btSphereShape* btSphere = new btSphereShape(max(halfSideLengths.x, max(halfSideLengths.y, halfSideLengths.z))); \
physicsModelData[thisPhysicsModelDataIndex].shape = new PhysicsShape(btSphere);

_MakePhysicsModelClassImplementation(BoundingSphere, 1, (bbox.start + bbox.end) / 2.0f, _GL_PhysicsModel_makeBoundingSphere(),,);

#define _GL_PhysicsModel_makeConvexHull_0() \
bbox.start *= (float)physicsModelData[thisPhysicsModelDataIndex].scale; bbox.end *= (float)physicsModelData[thisPhysicsModelDataIndex].scale; \
btConvexHullShape* btConvexHull = new btConvexHullShape();

#define _GL_PhysicsModel_makeConvexHull_1() \
unsigned int numVertices = dataSize / vertexSize; \
for (unsigned int j = 0u; j < numVertices; j++) { \
	\
	vec3 vertex = *((vec3*)(vertexData + vertexSize * j)); \
	btVector3 point = toBulletVector(vertex) * physicsModelData[thisPhysicsModelDataIndex].scale; \
	btConvexHull->addPoint(point); \
	\
}

#define _GL_PhysicsModel_makeConvexHull_2() \
btConvexHull->optimizeConvexHull(); \
physicsModelData[thisPhysicsModelDataIndex].shape = new PhysicsShape(btConvexHull);

_MakePhysicsModelClassImplementation(ConvexHull, 2, (btScalar)0.0, _GL_PhysicsModel_makeConvexHull_0(), _GL_PhysicsModel_makeConvexHull_1(), _GL_PhysicsModel_makeConvexHull_2());

#define _GL_PhysicsModel_makeMeshShape_0() \
bbox.start *= (float)physicsModelData[thisPhysicsModelDataIndex].scale; bbox.end *= (float)physicsModelData[thisPhysicsModelDataIndex].scale; \
btTriangleIndexVertexArray* bulletMesh = new btTriangleIndexVertexArray(); \
physicsModelData[thisPhysicsModelDataIndex].bulletVertices = new btVec3*[modelData[thisModelDataIndex].numMeshes]; \
physicsModelData[thisPhysicsModelDataIndex].bulletIndices = new int*[modelData[thisModelDataIndex].numMeshes];

#define _GL_PhysicsModel_makeMeshShape_1() \
unsigned int numVertices = dataSize / vertexSize; \
physicsModelData[thisPhysicsModelDataIndex].bulletVertices[i] = new btVec3[numVertices]; \
physicsModelData[thisPhysicsModelDataIndex].bulletIndices[i] = (int*)indexData; \
deleteIndices = false; \
\
for (unsigned int j = 0u; j < numVertices; j++) { \
	\
	vec3 vertex = *((vec3*)(vertexData + vertexSize * j)); \
	physicsModelData[thisPhysicsModelDataIndex].bulletVertices[i][j] = btVec3(vertex) * physicsModelData[thisPhysicsModelDataIndex].scale; \
	\
} \
\
btIndexedMesh bulletMeshEntry; \
bulletMeshEntry.m_numTriangles = modelData[thisModelDataIndex].numElements[i] / 3u; \
bulletMeshEntry.m_numVertices = numVertices; \
bulletMeshEntry.m_triangleIndexStride = 3u * sizeof(unsigned int); \
bulletMeshEntry.m_vertexStride = sizeof(btVec3); \
bulletMeshEntry.m_triangleIndexBase = (unsigned char*)physicsModelData[thisPhysicsModelDataIndex].bulletIndices[i]; \
bulletMeshEntry.m_vertexBase = (unsigned char*)physicsModelData[thisPhysicsModelDataIndex].bulletVertices[i]; \
bulletMesh->addIndexedMesh(bulletMeshEntry);

#define _GL_PhysicsModel_makeMeshShape_2() \
if (mass == (btScalar)0.0 && !isKinematic) physicsModelData[thisPhysicsModelDataIndex].shape = new PhysicsShape(new btBvhTriangleMeshShape(bulletMesh, true)); \
else { \
	\
	btGImpactMeshShape* bulletMeshShape = new btGImpactMeshShape(bulletMesh); \
	bulletMeshShape->updateBound(); \
	physicsModelData[thisPhysicsModelDataIndex].shape = new PhysicsShape(bulletMeshShape); \
	\
}

_MakePhysicsModelClassImplementation(GeneralPurpose, 3 + (int)(mass == (btScalar)0.0), (btScalar)0.0, _GL_PhysicsModel_makeMeshShape_0(), _GL_PhysicsModel_makeMeshShape_1(), _GL_PhysicsModel_makeMeshShape_2());

#endif