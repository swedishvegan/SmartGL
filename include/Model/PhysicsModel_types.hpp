#ifndef PHYSICSMODEL_TYPES_HPP
#define	PHYSICSMODEL_TYPES_HPP

#define _GL_PhysicsModel_declareHelperTypes() \
struct PhysicsModelData { \
	\
	const char* name; \
	unsigned int referenceCount = 0u; \
	\
	PhysicsShape* shape; \
	\
	btVec3** bulletVertices = nullptr; \
	int** bulletIndices = nullptr; \
	\
	int type; \
	btScalar scale; \
	\
};

#endif