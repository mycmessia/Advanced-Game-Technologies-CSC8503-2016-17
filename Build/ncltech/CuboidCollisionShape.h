/******************************************************************************
Class: CuboidCollisionShape
Implements: CollisionShape
Author: Pieran Marris      <p.marris@newcastle.ac.uk> and YOU!
Description: 

Extends CollisionShape to represent a cuboid geometric shape.

To simplify some of the methods such as getting the min/max vertices and acquiring the reference face, this 
class uses a cuboid 'Hull' object as it's basis. This keeps a list of all vertices, faces and their inter-connectivity,
allowing easy access to adjacent faces which is required for retrieving the clipping planes later on.



        (\_/)
        ( '_')
     /""""""""""""\=========     -----D
    /"""""""""""""""""""""""\
....\_@____@____@____@____@_/

*//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CollisionShape.h"
#include "Hull.h"

class CuboidCollisionShape : public CollisionShape
{
public:
	CuboidCollisionShape();
	CuboidCollisionShape(const Vector3& halfdims);
	virtual ~CuboidCollisionShape();

	// Set Cuboid Dimensions
	void SetHalfWidth(float half_width) { m_CuboidHalfDimensions.x = fabs(half_width); }
	void SetHalfHeight(float half_height) { m_CuboidHalfDimensions.y = fabs(half_height); }
	void SetHalfDepth(float half_depth) { m_CuboidHalfDimensions.z = fabs(half_depth); }

	// Get Cuboid Dimensions
	const Vector3& GetHalfDims() const { return m_CuboidHalfDimensions; }
	float GetHalfWidth()	const { return m_CuboidHalfDimensions.x; }
	float GetHalfHeight()	const { return m_CuboidHalfDimensions.y; }
	float GetHalfDepth()	const { return m_CuboidHalfDimensions.z; }

	// Debug Collision Shape
	virtual void DebugDraw(const PhysicsObject* currentObject) const override;


	// Build Inertia Matrix for rotational mass
	virtual Matrix3 BuildInverseInertia(float invMass) const override;


	// Generic Collision Detection Routines
	//  - Used in CollisionDetectionSAT to identify if two shapes overlap
	virtual void GetCollisionAxes(
		const PhysicsObject* currentObject,
		std::vector<Vector3>* out_axes) const override;

	virtual void GetEdges(
		const PhysicsObject* currentObject,
		std::vector<CollisionEdge>* out_edges) const override;

	virtual void GetMinMaxVertexOnAxis(
		const PhysicsObject* currentObject,
		const Vector3& axis,
		Vector3* out_min,
		Vector3* out_max) const override;

	virtual void GetIncidentReferencePolygon(
		const PhysicsObject* currentObject,
		const Vector3& axis,
		std::list<Vector3>* out_face,
		Vector3* out_normal,
		std::vector<Plane>* out_adjacent_planes) const override;



protected:
	//Constructs the static cube hull 
	static void ConstructCubeHull();

protected:
	Vector3				 m_CuboidHalfDimensions;
	static Hull			 m_CubeHull;			//Static cube descriptor, as all cuboid instances will have the same underlying model format
}; 

