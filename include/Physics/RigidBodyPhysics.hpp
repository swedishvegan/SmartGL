#ifndef RIGIDBODYPHYSICS_HPP
#define RIGIDBODYPHYSICS_HPP

#ifndef SmartGL_NO_PHYSICS

#include "./../util/GL-math.hpp"
#include "./../util/Exception.hpp"

#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#include <BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>

namespace GL {

	typedef Vector3D<btScalar> btVec3;
	typedef Matrix3x3<btScalar> btMat3;
	typedef Matrix4x4<btScalar> btMat4;

	struct CollisionData {

		unsigned int numContacts;

		btVec3 contacts[4];
		btVec3 normals[4];

	};

	class PhysicsObject;

	class PhysicsWorld {
	public:

		PhysicsWorld(btVec3 gravity);

		void setGravity(btVec3 gravity) const;

		void step(btScalar elapsedTime, btScalar rate = (btScalar)60) const;

		void getCollisionData(const PhysicsObject& objA, const PhysicsObject& objB, CollisionData& data) const;

		btDiscreteDynamicsWorld* getBulletPointer() const;

		PhysicsWorld(const PhysicsWorld&) = delete;
		void operator = (const PhysicsWorld&) = delete;

		~PhysicsWorld();

	protected:

		btDefaultCollisionConfiguration* collisionConfiguration;
		btCollisionDispatcher* dispatcher;
		btBroadphaseInterface* overlappingPairCache;
		btSequentialImpulseConstraintSolver* solver;

		btDiscreteDynamicsWorld* world;

	};

	class PhysicsShape {
	public:

		PhysicsShape(btCollisionShape* shape);

		btCollisionShape* getBulletPointer() const;
		
		PhysicsShape(const PhysicsShape&) = delete;
		void operator = (const PhysicsShape&) = delete;

		~PhysicsShape();

	protected:

		btCollisionShape* shape = nullptr;

	};

	class PhysicsObject {
	public:

		PhysicsObject(const PhysicsShape& shape, const PhysicsWorld& world, btScalar mass, bool isKinematic = false, btVec3 position = btVec3(), btMat3 rotation = btMat3());

		btRigidBody* getBulletPointer() const;

		btVec3 getPosition() const;

		btMat3 getRotation() const;

		btVec3 getVelocity() const;

		btVec3 getAngularVelocity() const;

		void setPosition(btVec3 position) const;

		void setRotation(btMat3 rotation) const;

		void teleport(btVec3 position, btMat3 rotation = btMat3()) const;

		void applyForce(btVec3 force) const;

		void applyForceImpulse(btVec3 impluse) const;

		void applyTorque(btVec3 torque) const;

		void applyTorqueImpulse(btVec3 impulse) const;

		void clearForces() const;

		void setVelocity(btVec3 velocity) const;

		void setAngularVelocity(btVec3 velocity) const;

		void setBounciness(btScalar bounciness) const;

		void setLinearDamping(btScalar linear);

		void setAngularDamping(btScalar angular);

		PhysicsObject(const PhysicsObject&) = delete;
		void operator = (const PhysicsObject&) = delete;

		~PhysicsObject();

	protected:

		btRigidBody* body;
		const PhysicsWorld* worldPtr;
		bool isStatic;

		btScalar linearDamping = 0.0;
		btScalar angularDamping = 0.0;

	};

	btMatrix3x3 toBulletMatrix(btMat3 mat);

	btVector3 toBulletVector(btVec3 vec);

	btMat3 fromBulletMatrix(btMatrix3x3 mat);

	btVec3 fromBulletVector(btVector3 vec);

}

#endif

#endif