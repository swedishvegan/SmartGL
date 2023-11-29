
#include "./RigidBodyPhysics.hpp"

#ifndef SmartGL_NO_PHYSICS

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

void GL::PhysicsWorld::step(btScalar elapsedTime, btScalar rate) const { world->stepSimulation(max(elapsedTime, (btScalar)0), 10, (btScalar)1 / rate); }

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

GL::PhysicsObject::PhysicsObject(const PhysicsShape& shape, const PhysicsWorld& world, btScalar mass, bool isKinematic, btVec3 position, btMat3 rotation) {
	
	if (isKinematic) mass = (btScalar)0;
	isStatic = (mass == (btScalar)0 && !isKinematic);

	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(toBulletVector(position));
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

	if (isKinematic) {

		body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
		body->setActivationState(DISABLE_DEACTIVATION);

	}

}

btRigidBody* GL::PhysicsObject::getBulletPointer() const { return body; }

GL::btVec3 GL::PhysicsObject::getPosition() const {

	btTransform transform;
	body->getMotionState()->getWorldTransform(transform);
	return fromBulletVector(transform.getOrigin());

}

GL::btMat3 GL::PhysicsObject::getRotation() const {

	btTransform transform;
	body->getMotionState()->getWorldTransform(transform);
	return fromBulletMatrix(transform.getBasis());

}

GL::btVec3 GL::PhysicsObject::getVelocity() const { return fromBulletVector(body->getLinearVelocity()); }

GL::btVec3 GL::PhysicsObject::getAngularVelocity() const { return fromBulletVector(body->getAngularVelocity()); }

void GL::PhysicsObject::setPosition(GL::btVec3 position) const {

	if (isStatic) throw Exception("A static physics object cannot be moved.");

	btTransform initialTransform;
	body->getMotionState()->getWorldTransform(initialTransform);
	initialTransform.setOrigin(toBulletVector(position));

	body->setWorldTransform(initialTransform);
	body->getMotionState()->setWorldTransform(initialTransform);

}

void GL::PhysicsObject::setRotation(GL::btMat3 rotation) const {

	if (isStatic) throw Exception("A static physics object cannot be rotated.");

	btTransform initialTransform;
	body->getMotionState()->getWorldTransform(initialTransform);
	initialTransform.setBasis(toBulletMatrix(rotation));

	body->setWorldTransform(initialTransform);
	body->getMotionState()->setWorldTransform(initialTransform);

}

void GL::PhysicsObject::teleport(GL::btVec3 position, GL::btMat3 rotation) const {

	if (isStatic) throw Exception("A static physics object cannot be teleported.");

	btTransform initialTransform;
	initialTransform.setIdentity();
	initialTransform.setOrigin(toBulletVector(position));
	initialTransform.setBasis(toBulletMatrix(rotation));

	body->setWorldTransform(initialTransform);
	body->getMotionState()->setWorldTransform(initialTransform);

	setVelocity(btVec3());
	setAngularVelocity(btVec3());
	clearForces();

}

void GL::PhysicsObject::applyForce(GL::btVec3 force) const { body->setActivationState(ACTIVE_TAG); body->applyCentralForce(toBulletVector(force)); }

void GL::PhysicsObject::applyForceImpulse(GL::btVec3 impulse) const { body->setActivationState(ACTIVE_TAG); body->applyCentralImpulse(toBulletVector(impulse)); }

void GL::PhysicsObject::applyTorque(GL::btVec3 torque) const { body->setActivationState(ACTIVE_TAG); body->applyTorque(toBulletVector(torque)); }

void GL::PhysicsObject::applyTorqueImpulse(GL::btVec3 impulse) const { body->setActivationState(ACTIVE_TAG); body->applyTorqueImpulse(toBulletVector(impulse)); }

void GL::PhysicsObject::clearForces() const { body->clearForces(); }

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