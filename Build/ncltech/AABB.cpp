#include "AABB.h"
#include <ncltech\NCLDebug.h>
#include <ncltech\Object.h>
#include <ncltech\PhysicsObject.h>

bool AABB::Contains (Vector3 point)
{
	auto left = (point.x >= this->left ());
	auto right = (point.x <= this->right ());
	auto top = (point.y <= this->top ());
	auto bottom = (point.y >= this->bottom ());
	auto front = (point.z <= this->front ());
	auto back = (point.z >= this->back ());

	return (left && right && top && bottom && back && front);
}

// Check if this Physics Object is in the AABB no matter a cuboid or a shpere
bool AABB::Contains (PhysicsObject* po)
{
	// Check if all vertices of the cube is in the AABB
	Vector3 center = po->GetPosition ();
	Matrix4 localTrans = po->GetAssociatedObject ()->GetLocalTransform ();
	Vector3 scale = Vector3 (localTrans[0], localTrans[5], localTrans[10]);
	
	float halfX = scale.x;
	float halfY = scale.y;
	float halfZ = scale.z;

	// Calc the 8 vertices of this cuboid
	Vector3 v1 = Vector3 (center.x - halfX, center.y - halfY, center.z - halfZ);
	Vector3 v2 = Vector3 (center.x + halfX, center.y - halfY, center.z - halfZ);
	Vector3 v3 = Vector3 (center.x + halfX, center.y + halfY, center.z - halfZ);
	Vector3 v4 = Vector3 (center.x - halfX, center.y + halfY, center.z - halfZ);

	Vector3 v5 = Vector3 (center.x - halfX, center.y - halfY, center.z + halfZ);
	Vector3 v6 = Vector3 (center.x + halfX, center.y - halfY, center.z + halfZ);
	Vector3 v7 = Vector3 (center.x + halfX, center.y + halfY, center.z + halfZ);
	Vector3 v8 = Vector3 (center.x - halfX, center.y + halfY, center.z + halfZ);

	return (Contains (v1) && Contains (v2) && Contains (v3) && Contains (v4) &&
			Contains (v5) && Contains (v6) && Contains (v7) && Contains (v8));
}

void AABB::Draw ()
{
	NCLDebug::DrawHairLine (position, Vector3 (position.x + size.x, position.y, position.z));
	NCLDebug::DrawHairLine (position, Vector3 (position.x, position.y + size.y, position.z));
	NCLDebug::DrawHairLine (position, Vector3 (position.x, position.y, position.z + size.z));

	NCLDebug::DrawHairLine (
		Vector3 (position.x + size.x, position.y, position.z + size.z), 
		Vector3 (position.x + size.x, position.y, position.z)
	);

	NCLDebug::DrawHairLine (
		Vector3 (position.x + size.x, position.y, position.z + size.z), 
		Vector3 (position.x, position.y, position.z + size.z)
	);

	NCLDebug::DrawHairLine (
		Vector3 (position.x + size.x, position.y, position.z), 
		Vector3 (position.x + size.x, position.y + size.y, position.z)
	);

	NCLDebug::DrawHairLine (
		Vector3 (position.x, position.y, position.z + size.z), 
		Vector3 (position.x, position.y + size.y, position.z + size.z)
	);

	NCLDebug::DrawHairLine (
		Vector3 (position.x + size.x, position.y, position.z + size.z), 
		Vector3 (position.x + size.x, position.y + size.y, position.z + size.z)
	);

	NCLDebug::DrawHairLine (
		Vector3 (position.x, position.y + size.y, position.z + size.z), 
		Vector3 (position.x + size.x, position.y + size.y, position.z + size.z)
	);

	NCLDebug::DrawHairLine (
		Vector3 (position.x + size.x, position.y + size.y, position.z), 
		Vector3 (position.x + size.x, position.y + size.y, position.z + size.z)
	);

	NCLDebug::DrawHairLine (
		Vector3 (position.x, position.y + size.y, position.z), 
		Vector3 (position.x + size.x, position.y + size.y, position.z)
	);

	NCLDebug::DrawHairLine (
		Vector3 (position.x, position.y + size.y, position.z), 
		Vector3 (position.x, position.y + size.y, position.z + size.z)
	);
}