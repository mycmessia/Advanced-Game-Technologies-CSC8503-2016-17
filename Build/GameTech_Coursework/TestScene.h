#pragma once

#include <nclgl\Mesh.h>
#include <ncltech\Scene.h>
#include <ncltech\NetworkBase.h>

class TestScene : public Scene
{
public:
	TestScene (const std::string& friendly_name);
	virtual ~TestScene ();

	virtual void OnInitializeScene ()		override;
	virtual void OnCleanupScene ()			override;
	virtual void OnUpdateScene (float dt)	override;

	void ProcessNetworkEvent (const ENetEvent& evnt);

	void CreateQuadBox ();

	void SendScoreToServer ();

protected:
	Vector4		status_color;
    Vector4		status_colour_header;

	int			bulletCounter;			// generate unique bullet name
	
	Vector3		origin;					// Vector3 (0.f, 0.f, 0.f)
	float		axisLength;				// length of axis x, y, z
	
	void		DrawAxis ();

	// Network
	Object*     m_pObj;

	NetworkBase m_Network;
	ENetPeer*	m_pServerConnection;
};