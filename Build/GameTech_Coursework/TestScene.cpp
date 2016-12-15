#include "TestScene.h"

#include <sstream>
#include <nclgl\Vector4.h>
#include <ncltech\PhysicsEngine.h>
#include <ncltech\DistanceConstraint.h>
#include <ncltech\SceneManager.h>
#include <ncltech\CommonUtils.h>
#include <nclgl\OBJMesh.h>
#include <../Build/Tuts_Physics/ObjectPlayer.h>
#include <ncltech\CuboidCollisionShape.h>

using namespace CommonUtils;

TestScene::TestScene(const std::string& friendly_name)
	: Scene(friendly_name)
	, m_pServerConnection(NULL)
	, m_pObj(NULL)
{
	status_color = Vector4 (1.0f, 1.0f, 1.0f, 1.0f);
    status_colour_header = Vector4 (0.8f, 0.9f, 1.0f, 1.0f);

	drawMode = 0;		// texture only
	isDrawModeChanged = false;
}

TestScene::~TestScene() {}


void TestScene::OnInitializeScene()
{
	//Initialize Client Network
	if (m_Network.Initialize(0))
	{
		NCLDebug::Log("Network: Initialized!");

		//Attempt to connect to the server on localhost:1234
		m_pServerConnection = m_Network.ConnectPeer(127, 0, 0, 1, 1234);
		NCLDebug::Log("Network: Attempting to connect to server.");
	}

	m_pObj = CommonUtils::BuildCuboidObject (
		"Server",
		Vector3 (0.0f, 15.0f, 0.0f),
		Vector3 (0.5f, 0.5f, 0.5f),
		true,									//Physics Enabled here Purely to make setting position easier via Physics()->SetPosition()
		0.0f,
		false,
		false,
		Vector4 (0.5f, 0.5f, 0.5f, 1.0f)
	);
	this->AddGameObject (m_pObj);

	origin = Vector3 (0.0f, 0.0f, 0.0f);
	axisLength = 10.0f;

	//Disable the physics engine (We will be starting this later!)
	PhysicsEngine::Instance()->SetPaused(false);
	PhysicsEngine::Instance ()->SetInCourseWork (true);

	//Set the camera position
	SceneManager::Instance()->GetCamera()->SetPosition(Vector3(15.0f, 10.0f, -15.0f));
	SceneManager::Instance()->GetCamera()->SetYaw(140.f);
	SceneManager::Instance()->GetCamera()->SetPitch(-20.f);

	const int pyramid_stack_height = 4;
	for (int y = 0; y < pyramid_stack_height; ++y)	
	{
		for (int x = 0; x <= y; ++x)
		{
			Vector4 colour = CommonUtils::GenColour (y * 0.2f, 1.0f);
			Object* cube = CommonUtils::BuildCuboidObject (
				"",
				Vector3 (12.0f + x - y * 0.5f, float (pyramid_stack_height - 1 - y) * 4.0f, 4.0f * x),
				Vector3 (0.5f, 0.5f, 0.5f),
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

	AddMeshPlayer ();

	//CreateQuadBox ();
}

void TestScene::OnCleanupScene ()
{
	//Just delete all created game objects 
	//  - this is the default command on any Scene instance so we don't really need to override this function here.
	Scene::OnCleanupScene (); 

	m_pObj = NULL; // Deleted in above function

	//Send one final packet telling the server we are disconnecting
	// - We are not waiting to resend this, so if it fails to arrive
	//   the server will have to wait until we time out naturally
	enet_peer_disconnect_now(m_pServerConnection, 0);

	//Release network and all associated data/peer connections
	m_Network.Release();
	m_pServerConnection = NULL;
}

void TestScene::OnUpdateScene (float dt)
{
	Scene::OnUpdateScene(dt);

	if (Window::GetKeyboard ()->KeyTriggered (KEYBOARD_M))
	{
		isDrawModeChanged = true;

		drawMode++;

		if (drawMode > 2) drawMode = 0;
	}

	switch (drawMode)
	{
		case 0:
		{
			if (isDrawModeChanged)
			{
				uint drawFlags = PhysicsEngine::Instance()->GetDebugDrawFlags();

				drawFlags ^= DEBUGDRAW_FLAGS_COLLISIONVOLUMES;

				drawFlags ^= DEBUGDRAW_FLAGS_COLLISIONNORMALS;

				PhysicsEngine::Instance()->SetDebugDrawFlags(drawFlags);

				PhysicsEngine::Instance ()->SetIsZeroTrans (
					!PhysicsEngine::Instance ()->GetIsZeroTrans ()
				);

				isDrawModeChanged = false;
			}

			break;
		}
		case 1:
		{
			if (isDrawModeChanged)
			{
				uint drawFlags = PhysicsEngine::Instance()->GetDebugDrawFlags();

				drawFlags ^= DEBUGDRAW_FLAGS_COLLISIONVOLUMES;

				drawFlags ^= DEBUGDRAW_FLAGS_COLLISIONNORMALS;

				PhysicsEngine::Instance()->SetDebugDrawFlags(drawFlags);

				isDrawModeChanged = false;
			}

			break;
		}
		case 2:
		{
			if (isDrawModeChanged)
			{
				PhysicsEngine::Instance ()->SetIsZeroTrans (
					!PhysicsEngine::Instance ()->GetIsZeroTrans ()
				);

				isDrawModeChanged = false;
			}
			break;
		}

	}

	//Update Network
	auto callback = std::bind(
		&TestScene::ProcessNetworkEvent,	// Function to call
		this,								// Associated class instance
		std::placeholders::_1);				// Where to place the first parameter
	m_Network.ServiceNetwork(dt, callback);

	//Add Debug Information to screen
	uint8_t ip1 = m_pServerConnection->address.host & 0xFF;
	uint8_t ip2 = (m_pServerConnection->address.host >> 8) & 0xFF;
	uint8_t ip3 = (m_pServerConnection->address.host >> 16) & 0xFF;
	uint8_t ip4 = (m_pServerConnection->address.host >> 24) & 0xFF;

	NCLDebug::DrawTextWs (m_pObj->Physics()->GetPosition() + Vector3(0.f, 0.5f, 0.f), 14.f, TEXTALIGN_CENTRE, Vector4(), 
		"Peer: %u.%u.%u.%u:%u", ip1, ip2, ip3, ip4, m_pServerConnection->address.port);
	
	NCLDebug::AddStatusEntry (status_color, "");
	NCLDebug::AddStatusEntry (status_colour_header, "Network Traffic");
	NCLDebug::AddStatusEntry (status_color, "    Incoming: %5.2fKbps    Outgoing: %5.2fKbps", m_Network.m_IncomingKb, m_Network.m_OutgoingKb);

	// Lets sun a little bit...
	Vector3 invLightDir = Matrix4::Rotation (15.f * dt, Vector3(0.0f, 1.0f, 0.0f)) * SceneManager::Instance()->GetInverseLightDirection();
	SceneManager::Instance ()->SetInverseLightDirection (invLightDir);

	// shot bullet ball
	if (Window::GetKeyboard ()->KeyTriggered (KEYBOARD_J))
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

	if (Window::GetKeyboard ()->KeyTriggered (KEYBOARD_T))
	{
		bool hasAtmosphere = PhysicsEngine::Instance ()->HasAtmosphere ();
		PhysicsEngine::Instance ()->SetHasAtmosphere (!hasAtmosphere);
	}

	if (Window::GetKeyboard ()->KeyTriggered (KEYBOARD_O))
	{
		PhysicsEngine::Instance ()->SetIsDrawOcTree (
			!PhysicsEngine::Instance ()->GetIsDrawOcTree ()
		);

		PhysicsEngine::Instance ()->SetIsUseOcTree (
			!PhysicsEngine::Instance ()->GetIsUseOcTree ()
		);
	}

	if (PhysicsEngine::Instance ()->GetIsDrawOcTree ())
	{
		PhysicsEngine::Instance ()->GetOcTreeRoot ()->Draw ();
	}

	DrawAxis ();

	SendScoreToServer ();
}

void TestScene::DrawAxis ()
{
	NCLDebug::DrawHairLineNDT (
		origin, 
		Vector3 (origin.x + axisLength, origin.y, origin.z),
		Vector4 (1.0f, 0.0f, 0.0f, 1.0f)
	);

	NCLDebug::DrawHairLineNDT (
		origin, 
		Vector3 (origin.x, origin.y + axisLength, origin.z),
		Vector4 (0.0f, 1.0f, 0.0f, 1.0f)
	);

	NCLDebug::DrawHairLineNDT (
		origin, 
		Vector3 (origin.x, origin.y, origin.z + axisLength),
		Vector4 (0.0f, 0.0f, 1.0f, 1.0f)
	);
}

void TestScene::ProcessNetworkEvent(const ENetEvent& evnt)
{
	switch (evnt.type)
	{
	//New connection request or an existing peer accepted our connection request
	case ENET_EVENT_TYPE_CONNECT:
		{
			if (evnt.peer == m_pServerConnection)
			{
				NCLDebug::Log("Network: Successfully connected to server!");

				//Send a 'hello' packet
				//char* text_data = "Hellooo!";
				//ENetPacket* packet = enet_packet_create(text_data, strlen(text_data) + 1, 0);
				//enet_peer_send(m_pServerConnection, 0, packet);
			}	
		}
		break;


	//Server has sent us a new packet
	case ENET_EVENT_TYPE_RECEIVE:
		{
			//if (evnt.packet->dataLength == sizeof(Vector3))
			//{
			//	Vector3 pos;
			//	memcpy(&pos, evnt.packet->data, sizeof(Vector3));
			//	m_pObj->Physics()->SetPosition(pos);
			//}
			//else
			//{
			//	NCLERROR("Recieved Invalid Network Packet!");
			//}
			int* all_ints = reinterpret_cast<int *>(evnt.packet->data);
			int num_ints = evnt.packet->dataLength / sizeof (int);

			if (num_ints)
			{
				printf ("----------High Score----------\n");

				for (int i = 0; i < num_ints; i++)
				{
					printf (" %d\t%d\n", (i + 1), all_ints[i]);
				}

				printf ("\n");
			}
		}
		break;


	//Server has disconnected
	case ENET_EVENT_TYPE_DISCONNECT:
		{
			NCLDebug::Log("Network: Server has disconnected!");
		}
		break;
	}
}

void TestScene::CreateQuadBox ()
{
	Vector3 EarthPos = Vector3 (0.0f, 0.0f, 0.0f);
	Vector4 color = Vector4 (1.0f, 0.6f, 0.0f, 1.0f);

	Object* CageBoard1 = CommonUtils::BuildQuadObject("B1",
		EarthPos + Vector3(-4, 10, 0),
		Vector3(2, 2, 0.0),
		true,
		0.0f,
		true,
		false,
		color);
	this->AddGameObject(CageBoard1);
	CageBoard1->Physics()->SetOrientation(Quaternion::EulerAnglesToQuaternion(0, 90, 0));

	Object* CageBoard2 = CommonUtils::BuildQuadObject("B2",
		EarthPos + Vector3(-8, 10, 0),
		Vector3(2, 2, 0.0),
		true,
		0.0f,
		true,
		false,
		color);
	this->AddGameObject(CageBoard2);
	CageBoard2->Physics()->SetOrientation(Quaternion::EulerAnglesToQuaternion(0, -90, 0));

	Object* CageBoard3 = CommonUtils::BuildQuadObject("B3",
		EarthPos + Vector3(-6, 10, 2),
		Vector3(2, 2, 0.0),
		true,
		0.0f,
		true,
		false,
		color);
	this->AddGameObject(CageBoard3);

	Object* CageBoard4 = CommonUtils::BuildQuadObject("B4",
		EarthPos + Vector3(-6, 10, -2),
		Vector3(2, 2, 0.0),
		true,
		0.0f,
		true,
		false,
		color);
	this->AddGameObject(CageBoard4);
	CageBoard4->Physics()->SetOrientation(Quaternion::EulerAnglesToQuaternion(0, 180, 0));

	Object* CageBoard5 = CommonUtils::BuildQuadObject("B5",
		EarthPos + Vector3(-6, 12, 0),
		Vector3(2, 2, 0.0),
		true,
		0.0f,
		true,
		false,
		color);
	this->AddGameObject(CageBoard5);
	CageBoard5->Physics()->SetOrientation(Quaternion::EulerAnglesToQuaternion(-90, 0, 0));

	Object* CageBoard6 = CommonUtils::BuildQuadObject("B6",
		EarthPos + Vector3(-6, 8, 0),
		Vector3(2, 2, 0.0),
		true,
		0.0f,
		true,
		false,
		color);
	this->AddGameObject(CageBoard6);
	CageBoard6->Physics()->SetOrientation(Quaternion::EulerAnglesToQuaternion(90, 0, 0));
}

void TestScene::SendScoreToServer ()
{
	if (PhysicsEngine::Instance ()->GetShotPoints () > 0)
	{
		// Send score to server
		int score = (int) PhysicsEngine::Instance ()->GetShotPoints ();

		ostringstream text_score;
		text_score << score;
		ENetPacket* packet = enet_packet_create (
			text_score.str().c_str(), 
			strlen(text_score.str().c_str()) + 1, 
			ENET_PACKET_FLAG_RELIABLE
		);
		enet_peer_send(m_pServerConnection, 0, packet);
	}
}

void TestScene::AddMeshPlayer ()
{
	m_MeshPlayer = new OBJMesh (MESHDIR"raptor.obj");
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

	// Create Player
	ObjectPlayer* player = new ObjectPlayer ("Player1");
	player->SetMesh (m_MeshPlayer, false);
	player->CreatePhysicsNode ();
	player->Physics ()->SetPosition (Vector3 (0.0f, 0.5f, 12.f));
	player->Physics ()->SetCollisionShape (new CuboidCollisionShape (Vector3 (0.5f, 0.5f, 1.0f)));
	player->SetBoundingRadius (1.0f);
	player->SetColour (Vector4 (1.0f, 1.0f, 1.0f, 1.0f));
	this->AddGameObject (player);
}