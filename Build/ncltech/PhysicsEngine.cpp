#include "PhysicsEngine.h"
#include "Object.h"
#include "CollisionDetectionSAT.h"
#include "NCLDebug.h"
#include <nclgl\Window.h>
#include <omp.h>


void PhysicsEngine::SetDefaults()
{
	m_IsInCourseWork = false;
	m_HasAtmosphere = false;
	m_isDrawOcTree = false;
	m_isUseOcTree = false;
	m_isZeroTrans = false;

	m_DebugDrawFlags = NULL;
	m_IsPaused = false;
	m_UpdateTimestep = 1.0f / 60.f;
	m_UpdateAccum = 0.0f;
	m_Gravity = Vector3(0.0f, -9.81f, 0.0f);
	m_DampingFactor = 0.999f;
}

PhysicsEngine::PhysicsEngine()
{
	SetDefaults();
}

PhysicsEngine::~PhysicsEngine()
{
	RemoveAllPhysicsObjects();
}

void PhysicsEngine::AddPhysicsObject(PhysicsObject* obj)
{
	m_PhysicsObjects.push_back(obj);
}

void PhysicsEngine::RemovePhysicsObject(PhysicsObject* obj)
{
	//Lookup the object in question
	auto found_loc = std::find(m_PhysicsObjects.begin(), m_PhysicsObjects.end(), obj);

	//If found, remove it from the list
	if (found_loc != m_PhysicsObjects.end())
	{
		m_PhysicsObjects.erase(found_loc);
	}
}

void PhysicsEngine::RemoveAllPhysicsObjects()
{
	//Delete and remove all constraints/collision manifolds
	for (Constraint* c : m_vpConstraints)
	{
		delete c;
	}
	m_vpConstraints.clear();

	for (Manifold* m : m_vpManifolds)
	{
		delete m;
	}
	m_vpManifolds.clear();


	//Delete and remove all physics objects
	// - we also need to inform the (possible) associated game-object
	//   that the physics object no longer exists
	for (PhysicsObject* obj : m_PhysicsObjects)
	{
		if (obj->m_pParent != NULL) obj->m_pParent->m_pPhysicsObject = NULL;
		delete obj;
	}
	m_PhysicsObjects.clear();
}


void PhysicsEngine::Update(float deltaTime)
{
	const int max_updates_per_frame = 5;

	if (!m_IsPaused)
	{
		m_UpdateAccum += deltaTime;
		for (int i = 0; (m_UpdateAccum >= m_UpdateTimestep) && i < max_updates_per_frame; ++i)
		{
			m_UpdateAccum -= m_UpdateTimestep;
			if (!m_IsPaused) UpdatePhysics(); //Additional check here incase physics was paused mid-update and the contents of the physics need to be displayed
		}

		if (m_UpdateAccum >= m_UpdateTimestep)
		{
			NCLERROR("Physics too slow to run in real time!");
			//Drop Time in the hope that it can continue to run in real-time
			m_UpdateAccum = 0.0f;
		}
	}
}


void PhysicsEngine::UpdatePhysics()
{
	if (m_IsInCourseWork)
	{
		m_ShotPoints = 0.0f;
	}

	for (Manifold* m : m_vpManifolds)
	{
		delete m;
	}
	m_vpManifolds.clear();

	for(auto* obj : m_PhysicsObjects)
	{
		obj->m_isColl = false;
	}

	//Check for collisions
	BroadPhaseCollisions();
	NarrowPhaseCollisions();

	//Solve collision constraints
	SolveConstraints();

	//Update movement
	for (PhysicsObject* obj : m_PhysicsObjects)
	{
		UpdatePhysicsObject(obj);
	}
}


void PhysicsEngine::SolveConstraints()
{
	//Optional step to allow constraints to 
	// precompute values based off current velocities 
	// before they are updated in the main loop below.
	for (Manifold* m : m_vpManifolds)		m->PreSolverStep(m_UpdateTimestep);
	for (Constraint* c : m_vpConstraints)	c->PreSolverStep(m_UpdateTimestep);

	// Solve all Constraints and Collision Manifolds
	//for (Manifold* m : m_vpManifolds)		m->ApplyImpulse();
	//for (Constraint* c : m_vpConstraints)	c->ApplyImpulse();
	for (size_t i = 0; i < SOLVER_ITERATIONS; ++i)
	{
		for (Manifold * m : m_vpManifolds)
		{
			m->ApplyImpulse(/*factor*/);
		}

		for (Constraint * c : m_vpConstraints)
		{
			c->ApplyImpulse();
		}
	}
}


void PhysicsEngine::UpdatePhysicsObject(PhysicsObject* obj)
{
	if (m_IsInCourseWork)
	{
		m_Gravity = Vector3 (0.0f, 0.0f, 0.0f) - obj->GetPosition ();
 		m_Gravity.Normalise ();
		m_Gravity = m_Gravity * 9.8f;

		float disFromEarth = (Vector3 (0.0f, 0.0f, 0.0f) - obj->GetPosition ()).LengthSquared ();

		if (disFromEarth  < 100.0f)
		{
			obj->m_isInAtmosphere = true;
		}
		else
		{
			obj->m_isInAtmosphere = false;
		}

		if (m_HasAtmosphere && obj->m_isInAtmosphere)
		{
			m_DampingFactor = 0.98f;

			if (obj->m_isBullet)
			{
				obj->m_pParent->SetColour (Vector4 (0.8f, 0.0f, 0.0f, 1.0f));
			}
		}
		else
		{
			m_DampingFactor = 0.999f;

			if (obj->m_isBullet)
			{
				obj->m_pParent->SetColour (Vector4 (0.5f, 1.0f, 0.5f, 1.0f));
			}
		}

		if (obj->m_pParent->GetName () == "Earth" || obj->m_pParent->GetName () == "Target")
		{
			m_DampingFactor = 1.0f;
		}

		if (m_isZeroTrans)
		{
			Vector4 colour = obj->m_pParent->GetColour ();
			if (colour.w != 0.0f)
			{
				obj->m_pParent->SetColour (Vector4 (colour.x, colour.y, colour.z, 0.0f));
			}
		}
		else
		{
			Vector4 colour = obj->m_pParent->GetColour ();
			obj->m_pParent->SetColour (Vector4 (colour.x, colour.y, colour.z, 1.0f));
		}
	}

	/* TUTORIAL 2 */
	if (obj->m_InvMass > 0.0f)
	{
		obj->m_LinearVelocity += m_Gravity * m_UpdateTimestep;
	}
	
	obj->m_LinearVelocity += obj->m_Force * obj->m_InvMass * m_UpdateTimestep;
	obj->m_LinearVelocity = obj->m_LinearVelocity * m_DampingFactor;

	obj->m_AngularVelocity += obj->m_InvInertia * obj->m_Torque * m_UpdateTimestep;
	obj->m_AngularVelocity = obj->m_AngularVelocity * m_DampingFactor;

	if (m_IsInCourseWork && obj->m_isSleep)
	{
		obj->m_LinearVelocity = Vector3 (0.0f, 0.0f, 0.0f);
		obj->m_AngularVelocity = Vector3 (0.0f, 0.0f, 0.0f);
	}

	obj->m_Position += obj->m_LinearVelocity * m_UpdateTimestep;

	obj->m_Orientation = obj->m_Orientation + obj->m_Orientation * (obj->m_AngularVelocity * m_UpdateTimestep * 0.5f);
	obj->m_Orientation.Normalise ();

	obj->m_wsTransformInvalidated = true;
}


void PhysicsEngine::BroadPhaseCollisions()
{
	m_BroadphaseCollisionPairs.clear();

	if (m_IsInCourseWork && m_isUseOcTree)
	{
		root->GenerateCPs (m_BroadphaseCollisionPairs);
	}
	else
	{
		PhysicsObject *m_pObj1, *m_pObj2;
		//	The broadphase needs to build a list of all potentially colliding objects in the world,
		//	which then get accurately assesed in narrowphase. If this is too coarse then the system slows down with
		//	the complexity of narrowphase collision checking, if this is too fine then collisions may be missed.


		//	Brute force approach.
		//  - For every object A, assume it could collide with every other object.. 
		//    even if they are on the opposite sides of the world.
		if (m_PhysicsObjects.size() > 0)
		{
			for (size_t i = 0; i < m_PhysicsObjects.size() - 1; ++i)
			{
				for (size_t j = i + 1; j < m_PhysicsObjects.size(); ++j)
				{
					m_pObj1 = m_PhysicsObjects[i];
					m_pObj2 = m_PhysicsObjects[j];

					//Check they both atleast have collision shapes

					if (m_pObj1->GetCollisionShape() != NULL
						&& m_pObj2->GetCollisionShape() != NULL)
					{
						CollisionPair cp;
						cp.pObjectA = m_pObj1;
						cp.pObjectB = m_pObj2;
						m_BroadphaseCollisionPairs.push_back(cp);
					}
				}
			}
		}
	}
}


void PhysicsEngine::NarrowPhaseCollisions()
{
	if (m_BroadphaseCollisionPairs.size() > 0)
	{
		//Collision data to pass between detection and manifold generation stages.
		CollisionData colData;				

		//Collision Detection Algorithm to use
		CollisionDetectionSAT colDetect;	

		// Iterate over all possible collision pairs and perform accurate collision detection
		for (size_t i = 0; i < m_BroadphaseCollisionPairs.size(); ++i)
		{
			CollisionPair& cp = m_BroadphaseCollisionPairs[i];

			CollisionShape *shapeA = cp.pObjectA->GetCollisionShape();
			CollisionShape *shapeB = cp.pObjectB->GetCollisionShape();

			colDetect.BeginNewPair(
				cp.pObjectA,
				cp.pObjectB,
				cp.pObjectA->GetCollisionShape(),
				cp.pObjectB->GetCollisionShape());

			//--TUTORIAL 4 CODE--
			// Detects if the objects are colliding - Seperating Axis Theorem
			if (colDetect.AreColliding(&colData))
			{
				//Draw collision data to the window if requested
				// - Have to do this here as colData is only temporary. 
				if (m_DebugDrawFlags & DEBUGDRAW_FLAGS_COLLISIONNORMALS)
				{
					NCLDebug::DrawPointNDT(colData._pointOnPlane, 0.1f, Vector4(0.5f, 0.5f, 1.0f, 1.0f));
					NCLDebug::DrawThickLineNDT(colData._pointOnPlane, colData._pointOnPlane - colData._normal * colData._penetration, 0.05f, Vector4(0.0f, 0.0f, 1.0f, 1.0f));
				}

				//Check to see if any of the objects have collision callbacks that dont want the objects to physically collide
				bool okA = cp.pObjectA->FireOnCollisionEvent(cp.pObjectA, cp.pObjectB);
				bool okB = cp.pObjectB->FireOnCollisionEvent(cp.pObjectB, cp.pObjectA);

				if (okA && okB)
				{
					if (m_IsInCourseWork)
					{
						Vector3 posObjA = cp.pObjectA->GetPosition ();
						Vector3 posObjB = cp.pObjectB->GetPosition ();

						if (cp.pObjectA->m_isBullet && 
							cp.pObjectB->m_isTarget && 
							cp.pObjectA->m_isHitTarget == false)
						{
							cp.pObjectA->m_isHitTarget = true;
							m_ShotPoints = CalcBulletPoints (posObjA, posObjB);
						}
						else if (cp.pObjectA->m_isTarget && 
								 cp.pObjectB->m_isBullet &&
								 cp.pObjectB->m_isHitTarget == false)
						{
							cp.pObjectB->m_isHitTarget = true;
							m_ShotPoints = CalcBulletPoints (posObjA, posObjB);
						}

						if (cp.pObjectA->m_isSleep && 
							cp.pObjectB->m_isSleep == false && 
							cp.pObjectB->m_isBullet)
						{
							cp.pObjectA->m_isSleep = false;
						}

						if (cp.pObjectB->m_isSleep && 
							cp.pObjectA->m_isSleep == false && 
							cp.pObjectA->m_isBullet)
						{
							cp.pObjectB->m_isSleep = false;
						}
					}

					cp.pObjectA->m_isColl = true;
					cp.pObjectB->m_isColl = true;
					//-- TUTORIAL 5 CODE --
					// Build full collision manifold that will also handle the collision response between the two objects in the solver stage
					Manifold* manifold = new Manifold();
					manifold->Initiate(cp.pObjectA, cp.pObjectB);

					// Construct contact points that form the perimeter of the collision manifold
					colDetect.GenContactPoints(manifold);

					// Add to list of manifolds that need solving
					m_vpManifolds.push_back(manifold);
				}
			}
		}

	}
}


void PhysicsEngine::DebugRender()
{
	// Draw all collision manifolds
	if (m_DebugDrawFlags & DEBUGDRAW_FLAGS_MANIFOLD)
	{
		for (Manifold* m : m_vpManifolds)
		{
			m->DebugDraw();
		}
	}

	// Draw all constraints
	if (m_DebugDrawFlags & DEBUGDRAW_FLAGS_CONSTRAINT)
	{
		for (Constraint* c : m_vpConstraints)
		{
			c->DebugDraw();
		}
	}

	// Draw all associated collision shapes
	if (m_DebugDrawFlags & DEBUGDRAW_FLAGS_COLLISIONVOLUMES)
	{
		for (PhysicsObject* obj : m_PhysicsObjects)
		{
			if (obj->GetCollisionShape() != NULL)
			{
				obj->GetCollisionShape()->DebugDraw(obj);
			}
		}
	}
}

float PhysicsEngine::CalcBulletPoints (Vector3 v1, Vector3 v2)
{
	Vector3 v = v1 - v2;
	float dis = v.LengthSquared ();
	return 10.0f * (4.0f - dis);
}

void PhysicsEngine::InitOcTree ()
{
	root = new OcTree (Vector3 (-10.f, -10.f, -10.f), 32.0f, m_PhysicsObjects);
	root->BulidOcTree ();
}