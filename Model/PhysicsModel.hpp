#ifndef PHYSICSMODEL_HPP
#define PHYSICSMODEL_HPP

#include <BulletCollision/Gimpact/btGImpactShape.h>

#include "Model/Model.hpp"
#include "Model/PhysicsModel_types.hpp"
#include "Physics/RigidBodyPhysics.hpp"

namespace GL {

	class PhysicsModel : public Model {
	public:

		PhysicsObject& getPhysicsObject() const;

		mat4 getModelMatrix() const;

		~PhysicsModel();

	protected:

		PhysicsModel(const char* name, int type);

		_GL_PhysicsModel_declareHelperTypes();

		PhysicsObject* po = nullptr;

		btScalar scale;
		btVec3 center;

		unsigned int thisPhysicsModelDataIndex;
		bool shouldConstructNewShape;

		static std::vector<PhysicsModelData> physicsModelData;

	};

#define _GL_MakePhysicsModelClass(shapeType) \
class shapeType ## PhysicsModel : public PhysicsModel { \
public: \
	\
	shapeType ## PhysicsModel(const char* filePath, const PhysicsWorld& world, btScalar scale = 1.0, btScalar mass = 0.0, btVec3 offset = btVec3(), btMat3 rotation = btMat3(), bool preCompilePrograms = false); \
	\
private: \
	\
	void loadMeshes(ReadBinaryFile& rbf); \
	\
	btScalar mass; \
	\
};

	_GL_MakePhysicsModelClass(BoundingBox);

	_GL_MakePhysicsModelClass(BoundingSphere);

	_GL_MakePhysicsModelClass(ConvexHull);

	_GL_MakePhysicsModelClass(GeneralPurpose);

}

std::vector<GL::PhysicsModel::PhysicsModelData> GL::PhysicsModel::physicsModelData;

GL::PhysicsObject& GL::PhysicsModel::getPhysicsObject() const { return *po; }

GL::mat4 GL::PhysicsModel::getModelMatrix() const { return GL::translate(po->getOffset() - center) * GL::upscale<btMat4>(po->getRotation()) * GL::scale(btVec3(scale)); }

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

GL::PhysicsModel::PhysicsModel(const char* name, int type) {
	
	thisPhysicsModelDataIndex = physicsModelData.size();
	shouldConstructNewShape = true;

	for (unsigned int i = 0u; i < physicsModelData.size(); i++)

		if (strcmp(name, physicsModelData[i].name) == 0) {
			
			if (type != physicsModelData[i].type) throw Exception("The engine does not allow loading two different physics models with the same source file ('" + std::string(name) + "') but different shapes.");
			
			thisPhysicsModelDataIndex = i;
			shouldConstructNewShape = false;
			break;

		}

	if (shouldConstructNewShape) {

		physicsModelData.push_back(PhysicsModelData{ });
		physicsModelData[thisPhysicsModelDataIndex].name = name;
		physicsModelData[thisPhysicsModelDataIndex].type = type;

	}

	physicsModelData[thisPhysicsModelDataIndex].referenceCount++;

}

#define _MakePhysicsModelClassImplementation(shapeType, typeCalculation, centerCalculation, physicsCode0, physicsCode1, physicsCode2) \
GL:: ## shapeType ## PhysicsModel:: ## shapeType ## PhysicsModel(const char* filePath, const PhysicsWorld& world, btScalar scale, btScalar mass, btVec3 offset, btMat3 rotation, bool preCompilePrograms) : PhysicsModel(filePath, typeCalculation) { \
	\
	isPhysicsModel = true; \
	this->scale = scale; \
	this->mass = mass; \
	\
	_GL_Model_constructor(); \
	\
	center = centerCalculation; \
	po = new PhysicsObject(*physicsModelData[thisPhysicsModelDataIndex].shape, world, mass, offset + center, rotation); \
	\
} \
\
void GL:: ## shapeType ## PhysicsModel::loadMeshes(ReadBinaryFile& rbf) { \
	\
	if (shouldConstructNewShape) { _GL_Model_loadMeshes(physicsCode0, physicsCode1, physicsCode2); } \
	else { _GL_Model_loadMeshes(,,); } \
	\
}

#define _GL_PhysicsModel_makeBoundingBox() \
bbox.start *= (float)scale; bbox.end *= (float)scale; \
btVector3 halfSideLengths = toBulletVector(abs(bbox.start - bbox.end) / 2.0f); \
btBoxShape* btBbox = new btBoxShape(halfSideLengths); \
physicsModelData[thisPhysicsModelDataIndex].shape = new PhysicsShape(btBbox);

_MakePhysicsModelClassImplementation(BoundingBox, 0, (bbox.start + bbox.end) / 2.0f, _GL_PhysicsModel_makeBoundingBox(),,);

#define _GL_PhysicsModel_makeBoundingSphere() \
bbox.start *= (float)scale; bbox.end *= (float)scale; \
btVec3 halfSideLengths = abs(bbox.start - bbox.end) / 2.0f; \
btSphereShape* btSphere = new btSphereShape(max(halfSideLengths.x, max(halfSideLengths.y, halfSideLengths.z))); \
physicsModelData[thisPhysicsModelDataIndex].shape = new PhysicsShape(btSphere);

_MakePhysicsModelClassImplementation(BoundingSphere, 1, (bbox.start + bbox.end) / 2.0f, _GL_PhysicsModel_makeBoundingSphere(),,);

#define _GL_PhysicsModel_makeConvexHull_0() \
bbox.start *= (float)scale; bbox.end *= (float)scale; \
btConvexHullShape* btConvexHull = new btConvexHullShape();

#define _GL_PhysicsModel_makeConvexHull_1() \
unsigned int numVertices = dataSize / vertexSize; \
for (unsigned int j = 0u; j < numVertices; j++) { \
	\
	vec3 vertex = *((vec3*)(vertexData + vertexSize * j)); \
	btVector3 point = toBulletVector(vertex) * scale; \
	btConvexHull->addPoint(point); \
	\
}

#define _GL_PhysicsModel_makeConvexHull_2() \
btConvexHull->optimizeConvexHull(); \
physicsModelData[thisPhysicsModelDataIndex].shape = new PhysicsShape(btConvexHull);

_MakePhysicsModelClassImplementation(ConvexHull, 2, (btScalar)0.0, _GL_PhysicsModel_makeConvexHull_0(), _GL_PhysicsModel_makeConvexHull_1(), _GL_PhysicsModel_makeConvexHull_2());

#define _GL_PhysicsModel_makeMeshShape_0() \
bbox.start *= (float)scale; bbox.end *= (float)scale; \
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
	physicsModelData[thisPhysicsModelDataIndex].bulletVertices[i][j] = btVec3(vertex) * scale; \
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
if (mass == (btScalar)0.0) physicsModelData[thisPhysicsModelDataIndex].shape = new PhysicsShape(new btBvhTriangleMeshShape(bulletMesh, true)); \
else { \
	\
	btGImpactMeshShape* bulletMeshShape = new btGImpactMeshShape(bulletMesh); \
	bulletMeshShape->updateBound(); \
	physicsModelData[thisPhysicsModelDataIndex].shape = new PhysicsShape(bulletMeshShape); \
	\
}

_MakePhysicsModelClassImplementation(GeneralPurpose, 3 + (int)(mass == (btScalar)0.0), (btScalar)0.0, _GL_PhysicsModel_makeMeshShape_0(), _GL_PhysicsModel_makeMeshShape_1(), _GL_PhysicsModel_makeMeshShape_2());

#endif