/******************************************************************************
Class: Phy2_Integration
Implements:
Author: Pieran Marris <p.marris@newcastle.ac.uk> and YOU!
Description:

Integration Demo Scene, fires a projectile into a target visually tracking it's
movement through time. If the integration scheme is perfect the ball should 
pass directly through the centre of the target. =]

If your like me, and have forgetten everything about integration, don't worry!
You can find a really nice little refresher here:
https://www.khanacademy.org/math/calculus-home/integration-applications-calc/rectilinear-motion-integral-calc/v/antiderivative-acceleration


::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::: IF YOUR BORED! :::
::::::::::::::::::::::
	1. Why not try integrating rotation instead of linear movement. Is it as accurate as
	   linear integration? 
	   
	2. Why not try implementing a second order integration scheme such as Runge Kutta 2 (Mid-Point method).
	   Other second order integrators that are commonly used include: leapfrog and verlet, however
	   they also require storing the data in a different format to what we expect in the following
	   tutorials, so maybe come back to these after the physics lectures have finished :)

	2. Why stop? Runge Kutta 4 (RK4) is commonly used integrator that people seem to generally
	  agree (at the time of writing) is as far down the 'complexity vs accuracy' tradeoff you
	  need to go in order to produce very pleasing results.

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
#include <ncltech\ObjectMesh.h>
#include <ncltech\CommonMeshes.h>
#include <ncltech\CommonUtils.h>
#include <ncltech\SphereCollisionShape.h>

class Phy2_Integration : public Scene
{
public:
	Phy2_Integration(const std::string& friendly_name)
		: Scene(friendly_name)
	{
		m_TargetTexture = SOIL_load_OGL_texture(
			TEXTUREDIR"target.tga",
			SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
			SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);

		glBindTexture(GL_TEXTURE_2D, m_TargetTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);

		SetWorldRadius(10.0f);
	}

	~Phy2_Integration()
	{
		if (m_TargetTexture)
		{
			glDeleteTextures(1, &m_TargetTexture);
			m_TargetTexture = NULL;
		}
	}

	virtual void OnInitializeScene() override
	{
		m_TrajectoryPoints.clear();

		PhysicsEngine::Instance()->SetPaused(false);

		SceneManager::Instance()->GetCamera()->SetPosition(Vector3(-6.25f, 2.0f, 10.0f));
		SceneManager::Instance()->GetCamera()->SetPitch(0.0f);
		SceneManager::Instance()->GetCamera()->SetYaw(0.0f);

		PhysicsEngine::Instance()->SetGravity(Vector3(0.0f, 0.0f, 0.0f));		//No Gravity
		PhysicsEngine::Instance()->SetDampingFactor(1.0f);						//No Damping



																				//Create Ground
		this->AddGameObject(CommonUtils::BuildCuboidObject(
			"Ground",								//Friendly ID/Name
			Vector3(-6.25f, -0.2f, 0.0f),			//Centre Position
			Vector3(10.0f, 0.1f, 2.f),				//Scale
			false,									//No Physics Yet
			0.0f,									//No Physical Mass Yet
			false,									//No Collision Shape 
			false,									//Not Dragable By the user
			Vector4(0.2f, 1.0f, 0.5f, 1.0f)));		//Color


		//Create Target
		ObjectMesh* target = new ObjectMesh("Target");
		target->SetMesh(CommonMeshes::Cube(), false);
		target->SetTexture(m_TargetTexture, false);
		target->SetLocalTransform(Matrix4::Translation(Vector3(0.1f, 2.0f, 0.0f)) * Matrix4::Scale(Vector3(0.1f, 2.0f, 2.f)));
		target->SetColour(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		target->SetBoundingRadius(4.0f);
		this->AddGameObject(target);


		//Create a projectile
		m_Sphere = new ObjectMesh("Sphere");
		m_Sphere->SetMesh(CommonMeshes::Sphere(), false);
		m_Sphere->SetLocalTransform(Matrix4::Scale(Vector3(0.5f, 0.5f, 0.5f)));
		m_Sphere->SetColour(Vector4(1.0f, 0.2f, 0.5f, 1.0f));
		m_Sphere->SetBoundingRadius(1.0f);

		m_Sphere->CreatePhysicsNode();
		m_Sphere->Physics()->SetInverseMass(1.f);
		
		this->AddGameObject(m_Sphere);

		ResetScene(PhysicsEngine::Instance()->GetUpdateTimestep());
	}

	void ResetScene(float timestep)
	{
		PhysicsEngine::Instance()->SetUpdateTimestep(timestep);
		PhysicsEngine::Instance()->SetPaused(false);
		m_TrajectoryPoints.clear();
		m_Sphere->Physics()->SetPosition(Vector3(-12.5f, 2.0f, 0.f));
		m_Sphere->Physics()->SetLinearVelocity(Vector3(0.f, 2.5f, 0.0f));
		m_Sphere->Physics()->SetForce(Vector3(1.f, -1.f, 0.0f));
	}

	virtual void OnUpdateScene(float dt) override
	{
		Scene::OnUpdateScene(dt);

		NCLDebug::AddStatusEntry(
			Vector4(1.0f, 0.9f, 0.8f, 1.0f),
			"Physics Timestep: %5.2fms [%5.2ffps]",
			PhysicsEngine::Instance()->GetUpdateTimestep() * 1000.0f,
			1.0f / PhysicsEngine::Instance()->GetUpdateTimestep()
			);

		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "Select Integration Timestep:");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "     1: 5fps");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "     2: 15fps");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "     3: 30fps");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "     4: 60fps");

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_1))	ResetScene(1.0f / 5.0f);
		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_2))	ResetScene(1.0f / 15.0f);
		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_3))	ResetScene(1.0f / 30.0f);
		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_4))	ResetScene(1.0f / 60.0f);

		if (!PhysicsEngine::Instance()->IsPaused())
		{
			m_TrajectoryPoints.push_back(m_Sphere->Physics()->GetPosition());
		}

		if (m_Sphere->Physics()->GetPosition().y < 0.0f)
		{
			PhysicsEngine::Instance()->SetPaused(true);
		}

		for (size_t i = 1; i < m_TrajectoryPoints.size(); i++)
		{
			NCLDebug::DrawThickLine(m_TrajectoryPoints[i - 1], m_TrajectoryPoints[i], 0.05f, Vector4(1.0f, 0.0f, 0.0f, 1.0f));
		}
	}

private:
	GLuint					m_TargetTexture;
	ObjectMesh*				m_Sphere;
	std::vector<Vector3>	m_TrajectoryPoints;
};