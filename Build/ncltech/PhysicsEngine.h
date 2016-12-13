/******************************************************************************
Class: PhysicsEngine
Implements:
Author: Pieran Marris      <p.marris@newcastle.ac.uk> and YOU!
Description:

The glue that brings all our physics dreams together. This class is provided
pretty much as is, as most of the sub-systems are tied in to this already
with a bit of debugging code to visualise various parts of the physics engine.

If you want

The general runtime consists of:
	- Update(float dt)
	  - UpdatePhysics()
	     - Broadphase Collision Detection
		   Quickly identifies possible collisions in between objects, hopefully
		   with world space partitioning systems like Octrees.. but currently just
		   builds a list colliding all objects with all other objects. (Hopefully you
		   can find some free time to fix this =] )

		 - Narrowphase Collision Detection
		   Takes the list provided by the broadphase collision detection and 
		   accurately collides all objects, building a collision manifold as
		   required. (Tutorial 4/5)

		 - Solve Constraints & Collisions
		   Solves all velocity constraints in the physics system, these include
		   both Collision Constraints (Tutorial 5,6) and misc world constraints
		   (Tutorial 3)

		 - Update Physics Objects
		   Moves all physics objects through time, updating positions/rotations
		   etc. (Tutorial 2)

		(\_/)
		( '_')
	 /""""""""""""\=========     -----D
	/"""""""""""""""""""""""\
....\_@____@____@____@____@_/

*//////////////////////////////////////////////////////////////////////////////

#pragma once
#include "TSingleton.h"
#include "PhysicsObject.h"
#include "Constraint.h"
#include "Manifold.h"
#include <vector>
#include <mutex>
#include "AABB.h"
#include "OcTree.h"

#define SOLVER_ITERATIONS 50

#ifndef FALSE
	#define FALSE	0
	#define TRUE	1
#endif

#define DEBUGDRAW_FLAGS_CONSTRAINT				0x1
#define DEBUGDRAW_FLAGS_MANIFOLD				0x2
#define DEBUGDRAW_FLAGS_COLLISIONVOLUMES		0x4
#define DEBUGDRAW_FLAGS_COLLISIONNORMALS		0x8


struct CollisionPair	//Forms the output of the broadphase collision detection
{
	PhysicsObject* pObjectA;
	PhysicsObject* pObjectB;
};

class PhysicsEngine : public TSingleton<PhysicsEngine>
{
	friend class TSingleton < PhysicsEngine > ;
public:
	//Reset Default Values like gravity/timestep - called when scene is switched out
	void SetDefaults();

	//Add/Remove Physics Objects
	void AddPhysicsObject(PhysicsObject* obj);
	void RemovePhysicsObject(PhysicsObject* obj);
	void RemoveAllPhysicsObjects(); //Delete all physics entities etc and reset-physics environment for new scene to be initialized

	//Add Constraints
	void AddConstraint(Constraint* c) { m_vpConstraints.push_back(c); }
	

	//Update Physics Engine
	void Update(float deltaTime);			//Remember DeltaTime is 'seconds' since last update not milliseconds
	
	//Debug draw all physics objects, manifolds and constraints
	void DebugRender();



	//Getters / Setters 
	bool IsPaused()						{ return m_IsPaused; }
	void SetPaused(bool paused)			{ m_IsPaused = paused; }

	uint GetDebugDrawFlags()			{ return m_DebugDrawFlags;  }
	void SetDebugDrawFlags(uint flags)  { m_DebugDrawFlags = flags; }
	
	void SetUpdateTimestep(float updateTimestep) { m_UpdateTimestep = updateTimestep; }
	float GetUpdateTimestep()			{ return m_UpdateTimestep; }

	const Vector3& GetGravity()			{ return m_Gravity; }
	void SetGravity(const Vector3& g)	{ m_Gravity = g; }

	float GetDampingFactor()			{ return m_DampingFactor; }
	void  SetDampingFactor(float d)		{ m_DampingFactor = d; }

	float GetDeltaTime()				{ return m_UpdateTimestep; }

	bool IsInCourseWork ()				{ return m_IsInCourseWork; }
	void SetInCourseWork (bool isInCW)	{ m_IsInCourseWork = isInCW; }

	float GetShotPoints ()				{ return m_ShotPoints; }

	bool HasAtmosphere	()				{ return m_HasAtmosphere; }
	void SetHasAtmosphere (bool b)		{ m_HasAtmosphere = b; }

	void InitOcTree ();
	OcTree* GetOcTreeRoot ()				{ return root; }

protected:
	PhysicsEngine();
	~PhysicsEngine();

	//The actual time-independant update function
	void UpdatePhysics();

	//Handles broadphase collision detection
	void BroadPhaseCollisions();

	//Handles narrowphase collision detection
	void NarrowPhaseCollisions();

	//Updates all physics objects position, orientation, velocity etc - Tutorial 2
	void UpdatePhysicsObject(PhysicsObject* obj);
	
	//Solves all physical constraints (constraints and manifolds)
	void SolveConstraints();

	float CalcBulletPoints (Vector3 v1, Vector3 v2);

protected:
	bool		m_IsInCourseWork;
	bool		m_HasAtmosphere;

	bool		m_IsPaused;
	float		m_UpdateTimestep, m_UpdateAccum;
	uint		m_DebugDrawFlags;

	Vector3		m_Gravity;
	float		m_DampingFactor;

	float		m_ShotPoints;

	std::vector<CollisionPair> m_BroadphaseCollisionPairs;

	std::vector<PhysicsObject*> m_PhysicsObjects;

	std::vector<Constraint*>	m_vpConstraints;		// Misc constraints applying to one or more physics objects
	std::vector<Manifold*>		m_vpManifolds;			// Contact constraints between pairs of objects

	OcTree* root;
};