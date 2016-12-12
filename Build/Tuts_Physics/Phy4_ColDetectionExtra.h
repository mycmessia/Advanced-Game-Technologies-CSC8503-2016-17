/******************************************************************************
Class: Phy4_ColDetectionExtra
Implements:
Author: Pieran Marris <p.marris@newcastle.ac.uk>
Description:

So...

Originally this was the going to be the ColDetection demo scene, however
it doesn't actually help debugging or visualising any collision detection
code. It probably should have been deleted with all the other scenes though,
if nothing else, it does provide a nice insight into how to exploit physics engines
for artificial intelligence. If you add a nice ray-tracing routine for AI 'sight'
you can build a pretty robust agent very quickly :)

Yep, it's here for AI purposes. It has absolutely nothing todo with how long
I may or may not have spent bashing my head against 3D modeling programs,
just trying to make a damn cuboid house and blobby garden.. and the pain I 
would suffer knowing it was all for nothing if it were deleted.


::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::: IF YOUR /VERY/ BORED! :::
:::::::::::::::::::::::::::::
	1.  Go! Go.. and see the beautiful 3D geometry that may or may not have 
	    cost me my PhD in terms of time wasted.  


::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::


		 (\_/)                      
		 ( '_')                     
	 /""""""""""""\=========     -----D	
	/"""""""""""""""""""""""\			  
....\_@____@____@____@____@_/              

*//////////////////////////////////////////////////////////////////////////////
#pragma once

#include <nclgl\OBJMesh.h>
#include <ncltech\Scene.h>
#include <ncltech\SceneManager.h>
#include <ncltech\PhysicsEngine.h>
#include <ncltech\NCLDebug.h>
#include <ncltech\ObjectMesh.h>
#include <ncltech\SphereCollisionShape.h>
#include <ncltech\CuboidCollisionShape.h>
#include <ncltech\CommonUtils.h>
#include "ObjectPlayer.h"


class Phy4_ColDetectionExtra : public Scene
{
public:
	Phy4_ColDetectionExtra(const std::string& friendly_name)
		: Scene(friendly_name)
		, m_MeshHouse(NULL)
		, m_MeshGarden(NULL)
	{
		glGenTextures(1, &m_whiteTexture);
		glBindTexture(GL_TEXTURE_2D, m_whiteTexture);
		int white_pixel = 0xFFFFFFFF;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, &white_pixel);

		m_MeshHouse = new OBJMesh(MESHDIR"house.obj");
		m_MeshGarden = new OBJMesh(MESHDIR"garden.obj");

		m_MeshPlayer = new OBJMesh(MESHDIR"raptor.obj");
		m_MeshPlayer->GenerateNormals();
		GLuint dTex;

		dTex = SOIL_load_OGL_texture(
			TEXTUREDIR"raptor.jpg",
			SOIL_LOAD_AUTO,
			SOIL_CREATE_NEW_ID,
			SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
		glBindTexture(GL_TEXTURE_2D, dTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		m_MeshPlayer->SetTexture(dTex);
	}

	virtual ~Phy4_ColDetectionExtra()
	{
		if (m_whiteTexture)
		{
			glDeleteTextures(1, &m_whiteTexture);
			m_whiteTexture = NULL;
		}

		if (m_MeshHouse)
		{
			m_MeshHouse->SetTexture(NULL);
			delete m_MeshHouse;
			m_MeshHouse = NULL;
		}

		if (m_MeshGarden)
		{
			m_MeshGarden->SetTexture(NULL);
			delete m_MeshGarden;
			m_MeshGarden = NULL;
		}

		if (m_MeshPlayer)
		{
			delete m_MeshPlayer;
			m_MeshPlayer = NULL;
		}

	}

	virtual void OnInitializeScene() override
	{
		PhysicsEngine::Instance()->SetDebugDrawFlags(DEBUGDRAW_FLAGS_COLLISIONNORMALS | DEBUGDRAW_FLAGS_COLLISIONVOLUMES);

		SceneManager::Instance()->GetCamera()->SetPosition(Vector3(-3.0f, 10.0f, 15.0f));
		SceneManager::Instance()->GetCamera()->SetYaw(-10.f);
		SceneManager::Instance()->GetCamera()->SetPitch(-30.f);

		//Create Ground
		this->AddGameObject(CommonUtils::BuildCuboidObject(
			"Ground",
			Vector3(0.0f, -1.001f, 0.0f),
			Vector3(20.0f, 1.0f, 20.0f),
			false,
			0.0f,
			false,
			false,
			Vector4(0.2f, 0.5f, 1.0f, 1.0f)));


		//Create Player
		ObjectPlayer* player = new ObjectPlayer("Player1");
		player->SetMesh(m_MeshPlayer, false);
		player->CreatePhysicsNode();
		player->Physics()->SetPosition(Vector3(0.0f, 0.5f, 0.0f));
		player->Physics()->SetCollisionShape(new CuboidCollisionShape(Vector3(0.5f, 0.5f, 1.0f)));
		player->SetBoundingRadius(1.0f);
		player->SetColour(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
		this->AddGameObject(player);


		//Create Some Objects
		{
			const Vector3 col_size = Vector3(2.0f, 2.f, 2.f);
			ObjectMesh* obj = new ObjectMesh("House");
			obj->SetLocalTransform(Matrix4::Translation(Vector3(0.0f, -0.71f, 0.0f)) * Matrix4::Scale(Vector3(2.0f, 2.0f, 2.f)));	//Translation here to move the mesh down, as it not centred on the origin *cry*
			obj->SetMesh(m_MeshHouse, false);
			obj->SetTexture(m_whiteTexture, false);
			obj->SetColour(Vector4(0.8f, 0.3f, 0.1f, 1.0f));
			obj->SetBoundingRadius(col_size.Length());
			obj->CreatePhysicsNode();
			obj->Physics()->SetPosition(Vector3(-5.0f, 2.f, -5.0f));
			obj->Physics()->SetCollisionShape(new CuboidCollisionShape(col_size));



			obj->Physics()->SetOnCollisionCallback([](PhysicsObject* self, PhysicsObject* collidingObject) {
				NCLDebug::Log("You are inside the house!");

				//Returns false if the collision should /not/ be handled with a standard collision response (tut 6)
				//Returns true to handle collision as normal
				return false;	
			});

			this->AddGameObject(obj);
		}

		{
			const Vector3 col_size = Vector3(2.0f, 0.5f, 2.f);
			ObjectMesh* obj = new ObjectMesh("Garden");
			obj->SetLocalTransform(Matrix4::Translation(Vector3(0.0f, -0.5f, 0.0f)) *Matrix4::Scale(Vector3(2.0f, 1.0f, 2.f))); //Translation here to move the mesh down, as it /also/ not centred on the origin *cry*
			obj->SetMesh(m_MeshGarden, false);
			obj->SetTexture(m_whiteTexture, false);
			obj->SetColour(Vector4(0.5f, 1.0f, 0.5f, 1.0f));
			obj->SetBoundingRadius(col_size.Length());
			obj->CreatePhysicsNode();
			obj->Physics()->SetPosition(Vector3(5.0f, 0.5f, -5.0f));
			obj->Physics()->SetCollisionShape(new CuboidCollisionShape(col_size));
			obj->Physics()->SetOnCollisionCallback([](PhysicsObject* self, PhysicsObject* collidingObject) {
				NCLDebug::Log("You are inside the garden!");

				//Returns false if the collision should /not/ be handled with a standard collision response (tut 6)
				//Returns true to handle collision as normal
				return false;
			});

			this->AddGameObject(obj);
		}

		//'Hidden' Physics Node (no visual mesh or renderable object attached)
		{
			PhysicsObject* obj = new PhysicsObject();
			obj->SetPosition(Vector3(5.0f, 1.0f, 0.0f));
			obj->SetCollisionShape(new SphereCollisionShape(1.0f));
			obj->SetOnCollisionCallback([obj](PhysicsObject* self, PhysicsObject* collidingObject) {

				NCLDebug::Log("You found the secret!");

				float r_x = 5.f * ((rand() % 200) / 100.f - 1.0f);
				float r_z = 3.f * ((rand() % 200) / 100.f - 1.0f);
				self->SetPosition(Vector3(r_x, 1.0f, r_z + 3.0f));

				//Returns false if the collision should /not/ be handled with a standard collision response (tut 6)
				//Returns true to handle collision as normal
				return false;
			});
			PhysicsEngine::Instance()->AddPhysicsObject(obj);
		}
	}


	virtual void OnUpdateScene(float dt) override
	{
		Scene::OnUpdateScene(dt);

		uint drawFlags = PhysicsEngine::Instance()->GetDebugDrawFlags();

		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "Physics:");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "     (Arrow Keys to Move Player)");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "     Draw Collision Volumes : %s (Press C to toggle)", (drawFlags & DEBUGDRAW_FLAGS_COLLISIONVOLUMES) ? "Enabled" : "Disabled");
		NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "     Draw Collision Normals : %s (Press N to toggle)", (drawFlags & DEBUGDRAW_FLAGS_COLLISIONNORMALS) ? "Enabled" : "Disabled");

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_C))
			drawFlags ^= DEBUGDRAW_FLAGS_COLLISIONVOLUMES;

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_N))
			drawFlags ^= DEBUGDRAW_FLAGS_COLLISIONNORMALS;

		PhysicsEngine::Instance()->SetDebugDrawFlags(drawFlags);
	}

private:
	OBJMesh *m_MeshHouse, *m_MeshGarden;
	GLuint	m_whiteTexture;
	OBJMesh* m_MeshPlayer;
};