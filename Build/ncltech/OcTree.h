/******************************************************************************
Class: OcTree
Implements: Yuchen Mei
Description: 
******************************************************************************/

#pragma once

#include <vector>

class AABB;
class PhysicsObject;
struct CollisionPair;

class OcTree
{
public:
	OcTree (Vector3 pos, float size, std::vector<PhysicsObject*> &v);
	~OcTree ();

	OcTree* CreateNode (AABB octant, std::vector<PhysicsObject*> &v);
	
	void BulidOcTree ();

	void Draw ();	
	
	void Delete ();

	void GenerateCPs (std::vector<CollisionPair> &cpList);

private:
	std::vector<PhysicsObject*> m_physicsObjects;
	OcTree*						m_childNodes[8];
	OcTree*						m_parent;
	AABB*						m_region;
};