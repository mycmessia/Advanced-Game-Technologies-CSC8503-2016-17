/******************************************************************************
Class: AABB Axis-Aligned Bounding Box
Implements: Yuchen Mei
Description: Use position and size to implement a Bounding Box not min and max 
as usual cause I assume that min is more close to origin and box only increase
to the positive dir of x, y, z. It is may not useful everywhere but here can do
some contributes in BroadPhaseCollisions in PhysicsEngine.
******************************************************************************/
#pragma once

#include <nclgl\Vector2.h>
#include <nclgl\Vector3.h>

class PhysicsObject;

// axis aligned bounding box for octree, all AABBs are cubes
class AABB
{
private:
	// size is the length of a single edge of the box
	// Since my box is a cube, actually I don't need the size of x, y, z cause they are equal 
	Vector3 size;		
	Vector3 position;	// position is the vertex which most closed to origin

	float AABB::left () const { return position.x; }
	float AABB::right () const { return (position.x + size.x); }
	float AABB::top () const { return position.y + size.y; }
	float AABB::bottom () const { return position.y; }
	float AABB::back () const { return position.z; }
	float AABB::front () const { return position.z + size.z; }

public:
	AABB (Vector3 p, float dimension)
	{
		position = p;
		size = Vector3 (dimension, dimension, dimension);
	}

	AABB (Vector3 p, Vector3 s) : position (p), size (s) {};
	~AABB () {};

	bool Contains (Vector3 point);
	bool Contains (PhysicsObject* po);

	void Draw ();

	Vector3 GetPosition () { return position; }
	Vector3 GetSize () { return size; }
};