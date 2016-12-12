#include "CubeRobot.h"

//cube is a static class variable, and so must be initialised outside of the 
//class...here's as good a place as any!
Mesh* CubeRobot::cube = NULL;

CubeRobot::CubeRobot(void)	{	
	if(!cube) {
		CreateCube();
	}
	SetMesh(cube);

	//Make the body
	SceneNode*body = new SceneNode(cube,Vector4(1,0,0,1));
	body->SetModelScale(Vector3(10,15,5));
	body->SetTransform(Matrix4::Translation(Vector3(0,35,0)));
	AddChild(body);

	//Add the head
	head = new SceneNode(cube,Vector4(0,1,0,1));
	head->SetModelScale(Vector3(5,5,5));
	head->SetTransform(Matrix4::Translation(Vector3(0,30,0)));
	body->AddChild(head);

	//Add the left arm
	leftArm = new SceneNode(cube,Vector4(0,0,1,1));
	leftArm->SetModelScale(Vector3(3,-18,3));
	leftArm->SetTransform(Matrix4::Translation(Vector3(-12,30,-1)));
	body->AddChild(leftArm);

	//Add the right arm
	rightArm = new SceneNode(cube,Vector4(0,0,1,1));
	rightArm->SetModelScale(Vector3(3,-18,3));
	rightArm->SetTransform(Matrix4::Translation(Vector3(12,30,-1)));
	body->AddChild(rightArm);

	//Add the left leg
	leftLeg = new SceneNode(cube,Vector4(0,0,1,1));
	leftLeg->SetModelScale(Vector3(3,-17.5,3));
	leftLeg->SetTransform(Matrix4::Translation(Vector3(-8,0,0)));
	body->AddChild(leftLeg);

	//Finally the right leg!
	rightLeg = new SceneNode(cube,Vector4(0,0,1,1));
	rightLeg->SetModelScale(Vector3(3,-17.5,3));
	rightLeg->SetTransform(Matrix4::Translation(Vector3(8,0,0)));
	body->AddChild(rightLeg);

	//Giant CubeRobot!
	//transform = Matrix4::Scale(Vector3(10,10,10));

	//The Scene Management Tutorial introduces these, as cheap culling tests
	body->SetBoundingRadius(15.0f);
	head->SetBoundingRadius(5.0f);

	leftArm->SetBoundingRadius(18.0f);
	rightArm->SetBoundingRadius(18.0f);

	leftLeg->SetBoundingRadius(18.0f);
	rightLeg->SetBoundingRadius(18.0f);
}

void CubeRobot::Update(float msec) {
	transform = transform * Matrix4::Rotation(msec / 10.0f,Vector3(0,1,0));

	head->SetTransform(Matrix4::Rotation(-msec / 10.0f,Vector3(0,1,0))* head->GetTransform());
	leftArm->SetTransform(leftArm->GetTransform() * Matrix4::Rotation(-msec / 10.0f,Vector3(1,0,0)));
	rightArm->SetTransform(rightArm->GetTransform() * Matrix4::Rotation(msec / 10.0f,Vector3(1,0,0)));

	SceneNode::Update(msec);
}