#include "TestScene.h"

#include <nclgl\Vector4.h>
#include <ncltech\PhysicsEngine.h>
#include <ncltech\DistanceConstraint.h>
#include <ncltech\SceneManager.h>
#include <ncltech\CommonUtils.h>

#include "AABB.h"

using namespace CommonUtils;

TestScene::TestScene(const std::string& friendly_name)
	: Scene(friendly_name)
{}

TestScene::~TestScene() {}


void TestScene::OnInitializeScene()
{
	//Disable the physics engine (We will be starting this later!)
	PhysicsEngine::Instance()->SetPaused(false);
	PhysicsEngine::Instance ()->SetInCourseWork (true);

	//Set the camera position
	SceneManager::Instance()->GetCamera()->SetPosition(Vector3(15.0f, 10.0f, -15.0f));
	SceneManager::Instance()->GetCamera()->SetYaw(140.f);
	SceneManager::Instance()->GetCamera()->SetPitch(-20.f);

	const int pyramid_stack_height = 2;
	for (int y = 0; y < pyramid_stack_height; ++y)	
	{
		for (int x = 0; x <= y; ++x)
		{
			Vector4 colour = CommonUtils::GenColour (y * 0.2f, 1.0f);
			Object* cube = CommonUtils::BuildCuboidObject (
				"",
				Vector3(x - y * 0.5f, 0.5f + float(pyramid_stack_height - 1 - y), -10.0f),
				Vector3(0.5f, 0.5f, 0.5f),
				true,
				1.f,
				true,
				false,
				colour);
			cube->Physics()->SetFriction (1.0f);
			cube->Physics()->SetIsSleep (true);
			//cube->Physics()->SetElasticity(0.0f);	
			this->AddGameObject (cube);
		}
	}

	//Example usage of Log and LogE inside NCLDebug functionality
	//NCLDebug::Log("This is a log entry");
	//NCLERROR("THIS IS AN ERROR!");

	Object* target = CommonUtils::BuildTargetCuboidObject (
		"Target",
		Vector3(0.0f, 6.8f, 0.0f),
		Vector3(0.2f, 2.0f, 2.0f),
		true,
		0.0f,
		true,
		false,
		Vector4 (1.0f, 1.0f, 1.0f, 1.0f)
	);
	target->Physics ()->SetAngularVelocity (Vector3 (0.0f, 1.0f, 0.0f));
	target->Physics ()->SetIsTarget (true);
	this->AddGameObject(target);

	Vector3 pos = Vector3 (0.0f, 0.0f, 0.0f);
	Object* earth = BuildSphereObject (
		"Earth",								// Optional: Name
		pos,									// Position
		5.0f,									// Half-Dimensions (Radius) also works on cube
		true,									// Physics Enabled?
		0.0f,									// Physical Mass (must have physics enabled)
		true,									// Physically Collidable (has collision shape)
		false,									// Dragable by user?
		Vector4 (0.0f, 0.6f, 0.9f, 1.0f));		// Render colour
	earth->Physics ()->SetAngularVelocity (Vector3 (0.0f, 1.0f, 0.0f));
	this->AddGameObject (earth);

	AABB firstAABB (Vector3 (-5.0f, -5.0f, -5.0f), 10.0f);
	NCLDebug::Log(firstAABB.Contains (earth->Physics ()) ? "Contain" : "Not Contain");
}

void TestScene::OnCleanupScene()
{
	//Just delete all created game objects 
	//  - this is the default command on any Scene instance so we don't really need to override this function here.
	Scene::OnCleanupScene(); 
}

void TestScene::OnUpdateScene(float dt)
{
	// Lets sun a little bit...
	Vector3 invLightDir = Matrix4::Rotation(15.f * dt, Vector3(0.0f, 1.0f, 0.0f)) * SceneManager::Instance()->GetInverseLightDirection();
	SceneManager::Instance()->SetInverseLightDirection(invLightDir);

	// shot bullet ball
	if (Window::GetKeyboard()->KeyTriggered (KEYBOARD_J))
	{
		Vector3 pos = SceneManager::Instance()->GetCamera()->GetPosition ();
		Matrix4 viewMtx = SceneManager::Instance()->GetCamera()->BuildViewMatrix();
		Vector3 viewDir = -Vector3(viewMtx[2], viewMtx[6], viewMtx[10]);
		string name = "bullet" + std::to_string (bulletCounter);
		Object* sphere = BuildSphereObject (
			name,									// Optional: Name
			pos,									// Position
			0.5f,									// Half-Dimensions
			true,									// Physics Enabled?
			1.0f,									// Physical Mass (must have physics enabled)
			true,									// Physically Collidable (has collision shape)
			false,									// Dragable by user?
			Vector4 (0.5f, 1.0f, 0.5f, 1.0f));		// Render colour
		sphere->Physics ()->SetLinearVelocity (viewDir * 10.0f);
		sphere->Physics ()->SetIsBullet (true);
		this->AddGameObject (sphere);
		bulletCounter++;
	}

	if (Window::GetKeyboard()->KeyTriggered (KEYBOARD_Z))
	{
		bool hasAtmosphere = PhysicsEngine::Instance ()->HasAtmosphere ();
		PhysicsEngine::Instance ()->SetHasAtmosphere (!hasAtmosphere);
	}
}