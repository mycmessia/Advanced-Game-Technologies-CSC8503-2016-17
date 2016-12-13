#pragma once

#include "AABB.h"
#include <ncltech\PhysicsObject.h>
#include <vector>

class OcTree
{
public:
	OcTree (Vector3 pos, float size, std::vector<PhysicsObject*> &v);
	~OcTree ();

	OcTree* CreateNode (AABB* octant, std::vector<PhysicsObject*> &v);
	void BulidOcTree ();

	void Draw ();

private:
	std::vector<PhysicsObject*> m_physicsObjects;
	OcTree* m_childNodes[8];
	AABB* m_region;

	void Delete ();
};