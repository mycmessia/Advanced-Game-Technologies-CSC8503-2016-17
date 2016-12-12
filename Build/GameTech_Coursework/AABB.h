#pragma once

#include <nclgl\Vector2.h>
#include <nclgl\Vector3.h>
#include <ncltech\SphereCollisionShape.h>
#include <ncltech\CuboidCollisionShape.h>

// axis aligned bounding box for octree, all AABBs are cubes
class AABB
{
private:
	Vector3 position;	// position is the vertex which most closed to origin
	Vector3 size;

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
};