/******************************************************************************
Class: Phy4_ColDetection
Implements:
Author: Pieran Marris <p.marris@newcastle.ac.uk> and YOU!
Description:

This demo scene creates a series of collision tests for the physics
engine to solve. The Sphere-Sphere, Sphere-Cuboid and Cuboid-Cuboid
tests should automatically be performed by the physics engine. 

Hopefully the visual collision data, showing penetration depth and collision
normal are correct for all collisions. =]



::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::: IF YOUR BORED! :::
::::::::::::::::::::::
	1. Why not try making your own collision shapes?
	   Note: Useful list of moments of inertia:  https://en.wikipedia.org/wiki/List_of_moments_of_inertia

			1. Ellipsoid
			   Based off the sphereCollisionShape, can you adapt it to take in
			   a half-width, half-height and half-depth instead to describe an ellipsoid
			   instead of a perfect sphere? You will need to use the world transform matrix
			   to convert axis-aligned w,h,d into current orientation of the object.

			2. Regular Tetrahedron
			   Based off the cuboidCollisionShape, can you adapt it to form a 
			   regular tetrahedron - where all 4 vertices are 'r' radius away from the 
			   centre of mass.

			3. Why stop?
			   Based off these techniques, you should be able to form any convex collision
			   shape. Why not have a look at what collision shapes are available on other
			   popular physics engines?
	
	2. We can only handle convex collision shapes. This unfortunately is a curse in 
	   all physics engines, it is possible to make a generic concave collision algorithm
	   however just our O(n^3) seperating axis theorem is slow enough so we need to get creative.
	   Why not try having a physics object that can have multiple convex collision shapes
	   that contain a relative offset. This way we could string them together to roughly approximate
	   the concave shape we are trying to display; for instance a tree with an ellipsoid for the leaves
	   and a cylinder for the trunk.
	  

::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::



		(\_/)
		( '_')
	 /""""""""""""\=========     -----D
	/"""""""""""""""""""""""\
....\_@____@____@____@____@_/

*//////////////////////////////////////////////////////////////////////////////
#pragma once

#include <ncltech\Scene.h>
#include <ncltech\SceneManager.h>
#include <ncltech\PhysicsEngine.h>
#include <ncltech\NCLDebug.h>
#include <ncltech\DistanceConstraint.h>
#include <ncltech\CommonUtils.h>

class Phy4_ColDetection : public Scene
{
public:
	Phy4_ColDetection(const std::string& friendly_name)
		: Scene(friendly_name)
		, m_AccumTime(0.0f)
		, m_Rotating(true)
	{
	}
	float m_AccumTime;
	bool m_Rotating;

	const Vector3 ss_pos = Vector3(-5.5f, 1.5f, -5.0f);
	const Vector3 sc_pos = Vector3(4.5f, 1.5f, -5.0f);
	const Vector3 cc_pos = Vector3(-0.5f, 1.5f, 5.0f);

	virtual void OnInitializeScene() override
	{
		SceneManager::Instance()->GetCamera()->SetPosition(Vector3(-3.0f, 4.0f, 10.0f));
		SceneManager::Instance()->GetCamera()->SetPitch(-20.f);
		PhysicsEngine::Instance()->SetDebugDrawFlags(DEBUGDRAW_FLAGS_COLLISIONNORMALS);

		m_AccumTime = 0.0f;
		m_Rotating = true;

		//Create Ground (..why not?)
		Object* ground = CommonUtils::BuildCuboidObject(
			"Ground",
			Vector3(0.0f, 0.0f, 0.0f),
			Vector3(20.0f, 1.0f, 20.0f),
			false,
			0.0f,
			false,
			false,
			Vector4(0.2f, 0.5f, 1.0f, 1.0f));

		this->AddGameObject(ground);


		//Create Sphere-Sphere Manifold Test
		{
			Object* sphere = CommonUtils::BuildSphereObject("orbiting_sphere1",
				ss_pos + Vector3(0.75f, 0.0f, 0.0f),	//Position leading to 0.25 meter overlap between spheres
				0.5f,									//Radius
				true,									//Has Physics Object
				0.0f,									//Infinite Mass
				true,									//Has Collision Shape
				false,									//Dragable by the user
				CommonUtils::GenColour(0.3f, 0.5f));	//Color
			this->AddGameObject(sphere);	

			this->AddGameObject(CommonUtils::BuildSphereObject("",
				ss_pos,									//Position
				0.5f,									//Radius
				true,									//Has Physics Object
				0.0f,									//Infinite Mass
				true,									//Has Collision Shape
				true,									//Dragable by the user
				CommonUtils::GenColour(0.55f, 1.0f)));	//Color
		}

		//Create Sphere-Cuboid Manifold Test
		{

			Object* sphere = CommonUtils::BuildSphereObject("orbiting_sphere2",
				sc_pos + Vector3(0.9f, 0.0f, 0.0f),		//Position leading to 0.1 meter overlap on faces, and more on diagonals
				0.5f,									//Radius
				true,									//Has Physics Object
				0.0f,									//Infinite Mass
				true,									//Has Collision Shape
				false,									//Dragable by the user
				CommonUtils::GenColour(0.3f, 0.5f));	//Color
			this->AddGameObject(sphere);

			this->AddGameObject(CommonUtils::BuildCuboidObject("",
				sc_pos,									//Position
				Vector3(0.5f, 0.5f, 0.5f),				//Half dimensions
				true,									//Has Physics Object
				0.0f,									//Infinite Mass
				true,									//Has Collision Shape
				true,									//Dragable by the user
				CommonUtils::GenColour(0.55f, 1.0f)));	//Color
		}

		//Create Cuboid-Cuboid Manifold Test
		{

			Object* cuboid = CommonUtils::BuildCuboidObject("rotating_cuboid1",
				cc_pos + Vector3(0.75f, 0.0f, 0.0f),	//Position leading to 0.25 meter overlap on faces, and more on diagonals
				Vector3(0.5f, 0.5f, 0.5f),				//Half dimensions
				true,									//Has Physics Object
				0.0f,									//Infinite Mass
				true,									//Has Collision Shape
				false,									//Dragable by the user
				CommonUtils::GenColour(0.3f, 0.5f));	//Color
			this->AddGameObject(cuboid);

			this->AddGameObject(CommonUtils::BuildCuboidObject("",
				cc_pos,									//Position
				Vector3(0.5f, 0.5f, 0.5f),				//Half dimensions
				true,									//Has Physics Object
				0.0f,									//Infinite Mass
				true,									//Has Collision Shape
				true,									//Dragable by the user
				CommonUtils::GenColour(0.55f, 1.0f)));	//Color
		}

	}


	virtual void OnUpdateScene(float dt) override
	{
		Scene::OnUpdateScene(dt);

		//Update Rotating Objects!
		if (m_Rotating)
		{
			m_AccumTime += dt;

			//Radius (ranges from 0.2-1.3 meters taking 6 seconds to complete cycle)
			float rscalar = (cosf(DegToRad(m_AccumTime * 60.f)) * 0.5f + 0.5f) * 1.1f + 0.2f;

			//Offset (Orbit around origin of radius 'rscalar' taking 30 seconds to complete orbit)
			Vector3 offset = Vector3(
				cosf(DegToRad(m_AccumTime * 12.f)) * rscalar,
				0.0f,
				sinf(DegToRad(m_AccumTime * 12.f)) * rscalar
				);

			//Default Colour (not colliding)
			Vector4 beforeCol = CommonUtils::GenColour(0.3f, 0.5f);
			Vector4 duringCol = CommonUtils::GenColour(0.9f, 0.2f);

			Object* orbiting_sphere1 = this->FindGameObject("orbiting_sphere1");
			if (orbiting_sphere1 != NULL)
			{
				orbiting_sphere1->Physics()->SetPosition(ss_pos + offset);
				orbiting_sphere1->SetColour(beforeCol);

				if(orbiting_sphere1->Physics()->IsColl ())
				{
					orbiting_sphere1->SetColour(duringCol);
				}
			}

			Object* orbiting_sphere2 = this->FindGameObject("orbiting_sphere2");
			if (orbiting_sphere2 != NULL)
			{
				orbiting_sphere2->Physics()->SetPosition(sc_pos + offset);
				orbiting_sphere2->SetColour(beforeCol);
				
				if(orbiting_sphere2->Physics()->IsColl ())
				{
					orbiting_sphere2->SetColour(duringCol);
				}
			}

			Object* rotating_cuboid1 = this->FindGameObject("rotating_cuboid1");
			if (rotating_cuboid1 != NULL)
			{
				rotating_cuboid1->Physics()->SetPosition(cc_pos + offset);
				rotating_cuboid1->SetColour(beforeCol);

				if(rotating_cuboid1->Physics()->IsColl ())
				{
					rotating_cuboid1->SetColour(duringCol);
				}
			}
		}


		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_B))
			m_Rotating = !m_Rotating;


		uint drawFlags = PhysicsEngine::Instance()->GetDebugDrawFlags();


		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "Physics:");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "     Draw Collision Normals : %s (Press N to toggle)", (drawFlags & DEBUGDRAW_FLAGS_COLLISIONNORMALS) ? "Enabled" : "Disabled");

		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "     Animation: %s (Press B to toggle)", (m_Rotating) ? "Enabled" : "Disabled");

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_N))
			drawFlags ^= DEBUGDRAW_FLAGS_COLLISIONNORMALS;

		PhysicsEngine::Instance()->SetDebugDrawFlags(drawFlags);

	}
};