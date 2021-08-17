#ifndef RIGIDBODYPHYSICS_HPP
#define RIGIDBODYPHYSICS_HPP

#include <util/GL-math.hpp>
#include <util/Exception.hpp>

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

		void step(btScalar elapsedTime) const;

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

		PhysicsObject(const PhysicsShape& shape, const PhysicsWorld& world, btScalar mass, btVec3 offset = btVec3(), btMat3 rotation = btMat3());

		btRigidBody* getBulletPointer() const;

		btVec3 getOffset();

		btMat3 getRotation();

		void applyForce(btVec3 force) const;

		void applyForceImpulse(btVec3 impluse) const;

		void applyTorque(btVec3 torque) const;

		void applyTorqueImpulse(btVec3 impulse) const;

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
		btTransform transform;
		const PhysicsWorld* worldPtr;

		btScalar linearDamping = 0.0;
		btScalar angularDamping = 0.0;

	};

	btMatrix3x3 toBulletMatrix(btMat3 mat);

	btVector3 toBulletVector(btVec3 vec);

	btMat3 fromBulletMatrix(btMatrix3x3 mat);

	btVec3 fromBulletVector(btVector3 vec);

}

GL::PhysicsWorld::PhysicsWorld(btVec3 gravity) {

	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	overlappingPairCache = new btDbvtBroadphase();
	solver = new btSequentialImpulseConstraintSolver();

	btGImpactCollisionAlgorithm::registerAlgorithm(dispatcher);

	world = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
	setGravity(gravity);

}

void GL::PhysicsWorld::setGravity(btVec3 gravity) const { world->setGravity(toBulletVector(gravity)); }

void GL::PhysicsWorld::step(btScalar elapsedTime) const { world->stepSimulation(max(elapsedTime, (btScalar)0), 10); }

void GL::PhysicsWorld::getCollisionData(const PhysicsObject& objA, const PhysicsObject& objB, CollisionData& data) const {

	data.numContacts = 0u;

	int numManifolds = world->getDispatcher()->getNumManifolds();
	for (int i = 0; i < numManifolds; i++) {

		btPersistentManifold* contactManifold = world->getDispatcher()->getManifoldByIndexInternal(i);
		const btCollisionObject* a = contactManifold->getBody0();
		const btCollisionObject* b = contactManifold->getBody1();

		if (a == (const btCollisionObject*)objA.getBulletPointer() && b == (const btCollisionObject*)objB.getBulletPointer()) {

			int numContacts = contactManifold->getNumContacts();
			data.numContacts = min(numContacts, 4);

			for (unsigned int j = 0u; j < data.numContacts; j++) {

				btManifoldPoint& pt = contactManifold->getContactPoint((int)j);

				data.contacts[j] = fromBulletVector(pt.getPositionWorldOnA());
				data.normals[j] = fromBulletVector(pt.m_normalWorldOnB);

			}

			break;

		}
	}
}

btDiscreteDynamicsWorld* GL::PhysicsWorld::getBulletPointer() const { return world; }

GL::PhysicsWorld::~PhysicsWorld() { 

	delete world;
	delete solver;
	delete overlappingPairCache;
	delete dispatcher;
	delete collisionConfiguration;

}

GL::PhysicsShape::PhysicsShape(btCollisionShape* shape) {

	if (!shape) throw Exception("PhysicsShape shape argument was nullptr.");
	this->shape = shape;

}

btCollisionShape* GL::PhysicsShape::getBulletPointer() const { return shape; }

GL::PhysicsShape::~PhysicsShape() { if (shape) delete shape; }

GL::PhysicsObject::PhysicsObject(const PhysicsShape& shape, const PhysicsWorld& world, btScalar mass, btVec3 offset, btMat3 rotation) {

	transform.setIdentity();
	transform.setOrigin(toBulletVector(offset));
	transform.setBasis(toBulletMatrix(rotation));
	mass = max(mass, (btScalar)0);

	btVector3 localInertia((btScalar)0, (btScalar)0, (btScalar)0);
	if (mass != (btScalar)0) shape.getBulletPointer()->calculateLocalInertia(mass, localInertia);

	btDefaultMotionState* motionState = new btDefaultMotionState(transform);
	btRigidBody::btRigidBodyConstructionInfo info(mass, motionState, shape.getBulletPointer(), localInertia);
	body = new btRigidBody(info);
	world.getBulletPointer()->addRigidBody(body);

	worldPtr = &world;
	body->setSleepingThresholds(0.01, 0.01);

}

btRigidBody* GL::PhysicsObject::getBulletPointer() const { return body; }

GL::btVec3 GL::PhysicsObject::getOffset() {

	body->getMotionState()->getWorldTransform(transform);
	return fromBulletVector(transform.getOrigin());

}

GL::btMat3 GL::PhysicsObject::getRotation() {

	body->getMotionState()->getWorldTransform(transform);
	return fromBulletMatrix(transform.getBasis());

}

void GL::PhysicsObject::applyForce(GL::btVec3 force) const { body->setActivationState(ACTIVE_TAG); body->applyCentralForce(toBulletVector(force)); }

void GL::PhysicsObject::applyForceImpulse(GL::btVec3 impulse) const { body->setActivationState(ACTIVE_TAG); body->applyCentralImpulse(toBulletVector(impulse)); }

void GL::PhysicsObject::applyTorque(GL::btVec3 torque) const { body->setActivationState(ACTIVE_TAG); body->applyTorque(toBulletVector(torque)); }

void GL::PhysicsObject::applyTorqueImpulse(GL::btVec3 impulse) const { body->setActivationState(ACTIVE_TAG); body->applyTorqueImpulse(toBulletVector(impulse)); }

void GL::PhysicsObject::setVelocity(btVec3 velocity) const { body->setActivationState(ACTIVE_TAG); body->setLinearVelocity(toBulletVector(velocity)); }

void GL::PhysicsObject::setAngularVelocity(btVec3 velocity) const { body->setActivationState(ACTIVE_TAG); body->setAngularVelocity(toBulletVector(velocity)); }

void GL::PhysicsObject::setBounciness(btScalar bounciness) const { body->setRestitution(bounciness); }

void GL::PhysicsObject::setLinearDamping(btScalar linear) { linearDamping = linear; body->setDamping(linear, angularDamping); }

void GL::PhysicsObject::setAngularDamping(btScalar angular) { angularDamping = angular; body->setDamping(linearDamping, angular); }

GL::PhysicsObject::~PhysicsObject() {

	delete body->getMotionState();
	worldPtr->getBulletPointer()->removeCollisionObject((btCollisionObject*)body);
	delete body;

}

btMatrix3x3 GL::toBulletMatrix(GL::btMat3 mat) {

	return btMatrix3x3(
		mat[0][0], mat[1][0], mat[2][0],
		mat[0][1], mat[1][1], mat[2][1],
		mat[0][2], mat[1][2], mat[2][2]
	);

}

btVector3 GL::toBulletVector(GL::btVec3 vec) { return btVector3(vec.x, vec.y, vec.z); }

GL::btMat3 GL::fromBulletMatrix(btMatrix3x3 mat) { return transpose(btMat3(mat)); }

GL::btVec3 GL::fromBulletVector(btVector3 vec) { return btVec3(vec); }

#endif