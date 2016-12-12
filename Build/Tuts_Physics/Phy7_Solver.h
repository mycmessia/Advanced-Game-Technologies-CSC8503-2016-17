/******************************************************************************
Class: Phy5_ColManifolds
Implements:
Author: Pieran Marris <p.marris@newcastle.ac.uk> and YOU!
Description:


Final physics demo scene demonstraing a pyramid of boxes stacked on top
of each other. 

If you have got this working, then:


          ___   ____
        /' --;^/ ,-_\     \ | /
       / / --o\ o-\ \\   --(_)--
      /-/-/|o|-|\-\\|\\   / | \
	 '`  ` |-|   `` '						..relax!
		   |-|						You have finished your 
		   |-|O						   physics engine!
		   |-(\,__
		...|-|\--,\_....
	,;;;;;;;;;;;;;;;;;;;;;;;;,.
~~,;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;,~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;,  ______   ---------   _____     ------ (From: http://ascii.co.uk/art/)




::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::: IF YOUR BORED! :::
::::::::::::::::::::::
    1. How high can you stack your boxes?
		You can improve the solver:
			1. Successive over relaxation is notionally better version of Gauss-Seidel
			   that we implemented. It utilises a factor 'g' that represents the amount
			   of force to update the constraints with. It has a starting value of 1 and
			   each iteration of the solver becomes x times smaller (where x is any number
			   between 0-1). The gauss-seidel that we implemented is actually just a 
			   SOR solver with a 'x' scalar of 1, always applying the full correctional force
			   each frame. If you want to improve the solver, this is a good place to start.

			2. The Jacobi method is a good one for parallisation on the gfx card, it is 
			   similar to Gauss-Seidel except each constraint is computed in parallel and
			   it's correctional force saved to an external buffer. This buffer is then
			   used later to apply all the constraint updates at once at the end of each
			   iteration. This does have a much lower convergence rate that other methods
			   however, as now the constraints cant apply correctional forces based on 
			   previous constraints until the next solver iteration but sits nicely on
			   CUDA/gpgpu computing.

		Constraint Order
			1. There is alot of cheats that can be enforced in certain conditions to
			   improve the convergence rate. For this stack of crates for instance,
			   solving the manifolds bottom to top will converge faster than top to 
			   bottom. You may want to consider sorting all your manifolds bottom-top
			   to improve stacking, or go further and implement a generic sorter based
			   relative velocity of set of conjoined manifolds to improve solver convergence.

			2. Constraint errors matter! By solving the same constraints in the same order
			   over multiple frames the errors will always be acumulated in the same direction
			   this can lead to a stack of boxes (very) slowly drifting to one side until it
			   topples and falls over. You may want to randomise the manifold order each physics
			   time step instead to mitigate these accumulated errors.

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
#include <ncltech\CommonUtils.h>
#include <ncltech\NCLDebug.h>
#include <ncltech\PhysicsEngine.h>

class Phy7_Solver : public Scene
{
public:
	Phy7_Solver(const std::string& friendly_name)
		: Scene(friendly_name)
	{}

	virtual void OnInitializeScene() override
	{
		SceneManager::Instance()->GetCamera()->SetPosition(Vector3(-2.0f, 7.5f, -8.0f));
		SceneManager::Instance()->GetCamera()->SetYaw(-160.f);
		SceneManager::Instance()->GetCamera()->SetPitch(-30.f);

		PhysicsEngine::Instance()->SetDebugDrawFlags(DEBUGDRAW_FLAGS_COLLISIONVOLUMES | DEBUGDRAW_FLAGS_MANIFOLD);

		//Create Ground
		this->AddGameObject(CommonUtils::BuildCuboidObject(
			"Ground",
			Vector3(0.0f, -1.0f, 0.0f),
			Vector3(20.0f, 1.0f, 20.0f),
			true,
			0.0f,
			true,
			false,
			Vector4(0.2f, 0.5f, 1.0f, 1.0f)));

		//SOLVER EXAMPLE -> Pyramid of cubes stacked on top of eachother
		{

			//This tower is purposely set to be built from top to bottom in order to give the solver a chalenge.
			// This will result in the first manifold (top) pushing all the second to top boxes downwards, eventually
			// reaching the bottom row and the ground and panicking because we now has to resolve the previous constraints 
			// in order to allow the bottom row of boxes to resolve their collisions positively in the y axis.
			//	-> This does also lead into a nice possible optimisation for stacking purposes where all the manifolds are
			//     sorted based on relative 'Y-axis' height, solving the constraints with the ground first and working its
			//     way up.
			const int pyramid_stack_height = 6;
			for (int y = 0; y < pyramid_stack_height; ++y)	
			{
				for (int x = 0; x <= y; ++x)
				{
					Vector4 colour = CommonUtils::GenColour(y * 0.2f, 1.0f);
					Object* cube = CommonUtils::BuildCuboidObject(
						"",
						Vector3(x - y * 0.5f, 0.5f + float(pyramid_stack_height - 1 - y), -0.5f),
						Vector3(0.5f, 0.5f, 0.5f),
						true,
						1.f,
						true,
						true,
						colour);
					cube->Physics()->SetFriction(1.0f);
					//cube->Physics()->SetElasticity(0.0f);	
					this->AddGameObject(cube);
				}
			}
		}

	}


	virtual void OnUpdateScene(float dt) override
	{
		Scene::OnUpdateScene(dt);

		uint drawFlags = PhysicsEngine::Instance()->GetDebugDrawFlags();

		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "Physics:");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "     Draw Collision Volumes : %s (Press C to toggle)", (drawFlags & DEBUGDRAW_FLAGS_COLLISIONVOLUMES) ? "Enabled" : "Disabled");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "     Draw Collision Normals : %s (Press N to toggle)", (drawFlags & DEBUGDRAW_FLAGS_COLLISIONNORMALS) ? "Enabled" : "Disabled");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "     Draw Manifolds : %s (Press M to toggle)", (drawFlags & DEBUGDRAW_FLAGS_MANIFOLD) ? "Enabled" : "Disabled");


		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_C))
			drawFlags ^= DEBUGDRAW_FLAGS_COLLISIONVOLUMES;

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_N))
			drawFlags ^= DEBUGDRAW_FLAGS_COLLISIONNORMALS;

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_M))
			drawFlags ^= DEBUGDRAW_FLAGS_MANIFOLD;

		PhysicsEngine::Instance()->SetDebugDrawFlags(drawFlags);
	}
};