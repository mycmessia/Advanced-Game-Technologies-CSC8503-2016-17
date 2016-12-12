/******************************************************************************
Class: Phy5_ColManifolds
Implements:
Author: Pieran Marris <p.marris@newcastle.ac.uk> and YOU!
Description:

This demo scene creates a series of collision tests, hopefully showing accurate
collision manifolds (surface area of the touching region). The three examples 
cover all the possibilities of sphere-cuboid collision shapes.



::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::: IF YOUR BORED! :::
::::::::::::::::::::::
	1.  The manifold at the moment stores a list of all points required to form it's
	    perimeter. However, for collision resolution (next tutorial), the more
		constraints the longer it will take to solve. One way to solve this
		is by using 'Persistant Manifolds', which both reuse existing contact
		points from previous frames (where valid), as well as managing the list
		of most important contact points - removing any that would just solve
		the same (or very simimar) resolution calculation. Key things to 
		improve the accuracy of the physics manifolds and conllision
		resolution in general are:

			- Reusing old contact points from the previous frame
				If the penetration distance is still negative, then why not
				reuse them? The more possible contact points the better as 
				it provides us with a greater insight into the full collision
				manifold from multiple faces. 
			Note: If you wanted to try implementing	GJK/EPA (better collision
				detection algorithm mentioned previously)  then this is MANDITORY!
				GJK/EPA will only ever generate a single contact point each physics
				timestep. The usual approach, is to let	the objects overlap eachother
				on multiple frames until a full	manifold is created with three+ points
				of contact.

			- Managing Contact Points - Dropping any deemed unworthy
				If we have two contact points at the same location they will
				end up solving the same distance constraint twice.. this is bad!
				After the global solver is implemented (last tutorial) this will
				result in a much slower convergence rate, and before that it can
				lead to explosions with too much energy continually added to the same
				point of contact. To solve this we need to manage the list of manifold
				contact points to ensure they both encompass all significant points of
				contact (upon solving no resolution forces are ignored) and also never
				have more contact points than the minima required to satisfy the previous
				condition.

				If your interested, bullet uses a quick method based on approximate
				contact weighting to never exceed 4 contact points and culls any that
				it doesn't believe makes a difference to the resolution step. Though
				there is any number of possible	ways of doing this, you could even form
				new contact points that are aconglomeration of existing surrounding
				contacting points if you wanted :)

				Bullet Header: http://bulletphysics.org/Bullet/BulletFull/btPersistentManifold_8h_source.html
				Bullet Source: http://bulletphysics.org/Bullet/BulletFull/btPersistentManifold_8cpp_source.html


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

class Phy5_ColManifolds : public Scene
{
public:
	Phy5_ColManifolds(const std::string& friendly_name)
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
		PhysicsEngine::Instance()->SetDebugDrawFlags(DEBUGDRAW_FLAGS_MANIFOLD);

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

			this->AddGameObject(CommonUtils::BuildSphereObject("orbiting_sphere1",
				ss_pos + Vector3(0.75f, 0.0f, 0.0f),	//Position leading to 0.25 meter overlap between spheres
				0.5f,									//Radius
				true,									//Has Physics Object
				0.0f,									//Infinite Mass
				true,									//Has Collision Shape
				false,									//Dragable by the user
				CommonUtils::GenColour(0.45f, 0.5f)));	//Color

			this->AddGameObject(CommonUtils::BuildSphereObject("",
				ss_pos,									//Position
				0.5f,									//Radius
				true,									//Has Physics Object
				0.0f,									//Infinite Mass
				true,									//Has Collision Shape
				true,									//Dragable by the user
				CommonUtils::GenColour(0.5f, 1.0f)));	//Color
		}

		//Create Sphere-Cuboid Manifold Test
		{

			this->AddGameObject(CommonUtils::BuildSphereObject("orbiting_sphere2",
				sc_pos + Vector3(0.9f, 0.0f, 0.0f),		//Position leading to 0.1 meter overlap on faces, and more on diagonals
				0.5f,									//Radius
				true,									//Has Physics Object
				0.0f,									//Infinite Mass
				true,									//Has Collision Shape
				false,									//Dragable by the user
				CommonUtils::GenColour(0.45f, 0.5f)));	//Color

			this->AddGameObject(CommonUtils::BuildCuboidObject("",
				sc_pos,									//Position
				Vector3(0.5f, 0.5f, 0.5f),				//Half dimensions
				true,									//Has Physics Object
				0.0f,									//Infinite Mass
				true,									//Has Collision Shape
				true,									//Dragable by the user
				CommonUtils::GenColour(0.5f, 1.0f)));	//Color
		}

		//Create Cuboid-Cuboid Manifold Test
		{
			
			this->AddGameObject(CommonUtils::BuildCuboidObject("rotating_cuboid1",
				cc_pos + Vector3(0.75f, 0.0f, 0.0f),	//Position leading to 0.25 meter overlap on faces, and more on diagonals
				Vector3(0.5f, 0.5f, 0.5f),				//Half dimensions
				true,									//Has Physics Object
				0.0f,									//Infinite Mass
				true,									//Has Collision Shape
				false,									//Dragable by the user
				CommonUtils::GenColour(0.45f, 0.5f)));	//Color

			this->AddGameObject(CommonUtils::BuildCuboidObject("",
				cc_pos,									//Position
				Vector3(0.5f, 0.5f, 0.5f),				//Half dimensions
				true,									//Has Physics Object
				0.0f,									//Infinite Mass
				true,									//Has Collision Shape
				true,									//Dragable by the user
				CommonUtils::GenColour(0.5f, 1.0f)));	//Color
		}

	}

	
	virtual void OnUpdateScene(float dt) override
	{
		Scene::OnUpdateScene(dt);
		

		//Update Rotating Objects!
		if (m_Rotating)
		{
			m_AccumTime += dt;

			Object* orbiting_sphere1 = this->FindGameObject("orbiting_sphere1");
			if (orbiting_sphere1 != NULL)
			{
				//Move orbiting sphere1 around centre object at 45 degrees per second with an orbiting radius of 75cm
				orbiting_sphere1->Physics()->SetPosition(Vector3(
					ss_pos.x + cos(DegToRad(m_AccumTime * 45.0f) * 2.f) * 0.75f,
					ss_pos.y,
					ss_pos.z + sin(DegToRad(m_AccumTime * 45.0f) * 2.f) * 0.75f));
			}

			Object* orbiting_sphere2 = this->FindGameObject("orbiting_sphere2");
			if (orbiting_sphere2 != NULL)
			{
				//Move orbiting sphere2 around centre object at 45 degrees per second with an orbiting radius of 90cm
				orbiting_sphere2->Physics()->SetPosition(Vector3(
					sc_pos.x + cos(DegToRad(m_AccumTime * 45.0f) * 2.f) * 0.9f,
					sc_pos.y,
					sc_pos.z + sin(DegToRad(m_AccumTime * 45.0f) * 2.f) * 0.9f));
			}

			Object* rotating_cuboid1 = this->FindGameObject("rotating_cuboid1");
			if (rotating_cuboid1 != NULL)
			{
				rotating_cuboid1->Physics()->SetOrientation(Quaternion::AxisAngleToQuaterion(Vector3(1.0f, 0.0f, 0.0f), m_AccumTime * 45.0f));
			}
		}


		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_B))
			m_Rotating = !m_Rotating;



		uint drawFlags = PhysicsEngine::Instance()->GetDebugDrawFlags();


		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "Physics:");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "     Draw Collision Normals : %s (Press N to toggle)", (drawFlags & DEBUGDRAW_FLAGS_COLLISIONNORMALS) ? "Enabled" : "Disabled");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "     Draw Manifolds : %s (Press M to toggle)", (drawFlags & DEBUGDRAW_FLAGS_MANIFOLD) ? "Enabled" : "Disabled");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "     Animation: %s (Press B to toggle)", (m_Rotating) ? "Enabled" : "Disabled");

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_N))
			drawFlags ^= DEBUGDRAW_FLAGS_COLLISIONNORMALS;

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_M))
			drawFlags ^= DEBUGDRAW_FLAGS_MANIFOLD;

		PhysicsEngine::Instance()->SetDebugDrawFlags(drawFlags);

	}
};