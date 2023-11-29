#ifndef PHYSICSMODEL_HPP
#define PHYSICSMODEL_HPP

#ifndef SmartGL_NO_PHYSICS

#include <BulletCollision/Gimpact/btGImpactShape.h>

#include "./Model.hpp"
#include "./PhysicsModel_types.hpp"
#include "./../Physics/RigidBodyPhysics.hpp"

namespace GL {

	class PhysicsModel : public Model {
	public:

		PhysicsObject& getPhysicsObject() const;

		mat4 getModelMatrix() const;

		~PhysicsModel();

	protected:

		PhysicsModel(const char* name, int type, btScalar scale);

		_GL_PhysicsModel_declareHelperTypes();

		PhysicsObject* po = nullptr;
		
		btVec3 center;
		BoundingBox bbox;

		unsigned int thisPhysicsModelDataIndex;
		bool shouldConstructNewShape;

		static std::vector<PhysicsModelData> physicsModelData;

	};

#define _GL_MakePhysicsModelClass(shapeType) \
class shapeType ## PhysicsModel : public PhysicsModel { \
public: \
	\
	shapeType ## PhysicsModel(const char* filePath, const PhysicsWorld& world, btScalar scale = 1.0, btScalar mass = 0.0, bool isKinematic = false, btVec3 offset = btVec3(), btMat3 rotation = btMat3(), bool preCompilePrograms = false); \
	\
private: \
	\
	void loadMeshes(ReadBinaryFile& rbf); \
	\
	btScalar mass; \
	bool isKinematic; \
	\
};

	_GL_MakePhysicsModelClass(BoundingBox);

	_GL_MakePhysicsModelClass(BoundingSphere);

	_GL_MakePhysicsModelClass(ConvexHull);

	_GL_MakePhysicsModelClass(GeneralPurpose);

}

#endif

#endif