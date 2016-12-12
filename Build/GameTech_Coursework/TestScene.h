#pragma once

#include <nclgl\Mesh.h>
#include <ncltech\Scene.h>
#include "OcTree.h"

class TestScene : public Scene
{
public:
	TestScene(const std::string& friendly_name);
	virtual ~TestScene();

	virtual void OnInitializeScene()	 override;
	virtual void OnCleanupScene()		 override;
	virtual void OnUpdateScene(float dt) override;

protected:
	int bulletCounter;

	Vector3 origin;
	float axisLength;

	OcTree* root;
	vector<PhysicsObject*> poVector; 

	void DrawAxis ();
};