/******************************************************************************
Class:Frustum
Implements:
Author:Rich Davison	<richard.davison4@newcastle.ac.uk>
Description: A view frustum, made up of 6 planes. Contains function to test
whether a SceneNode is inside the view frustum, via a simple bounding sphere
test.

Students! Once you get beyond bounding spheres and implement bounding boxes,
you'll probably want to turn the frustum test functions 'inside out' - so
that a frustum is passed to a SceneNode and tested against by its bounding 
volume class - this way the Frustum class doesn't need to know about whatever
bounding volumes you come up with. You'll need a way of accessing the
Frustum's Planes safely though...

//plane 0 = right
//plane 1 = left
//plane 2 = bottom
//plane 3 = top
//plane 4 = far
//plane 5 = near

-_-_-_-_-_-_-_,------,   
_-_-_-_-_-_-_-|   /\_/\   NYANYANYAN
-_-_-_-_-_-_-~|__( ^ .^) /
_-_-_-_-_-_-_-""  ""   

*//////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Plane.h"
#include "Matrix4.h"
#include "SceneNode.h"

class Frustum	{
public:
	Frustum(void){};
	~Frustum(void){};

	//Creates the frustum planes from a 'view projection' matrix
	void FromMatrix(const Matrix4 &viewProj);
	//Is a SceneNode inside this frustum?

	bool InsideFrustum(const Vector3& position, float radius) const;
	bool InsideFrustum(SceneNode&n) const;

	bool	AABBInsideFrustum(Vector3 &position, const Vector3 &size) const;

	Plane& GetPlane(int idx) { return planes[idx]; }

protected:
	Plane planes[6];
};
