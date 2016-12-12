#include "SphereCollisionShape.h"
#include "PhysicsObject.h"
#include "NCLDebug.h"
#include <nclgl/Matrix3.h>

SphereCollisionShape::SphereCollisionShape()
{
	m_Radius = 1.0f;
}

SphereCollisionShape::SphereCollisionShape(float radius)
{
	m_Radius = radius;
}

SphereCollisionShape::~SphereCollisionShape()
{

}

Matrix3 SphereCollisionShape::BuildInverseInertia(float invMass) const
{
	//https://en.wikipedia.org/wiki/List_of_moments_of_inertia
	float i = 2.5f * invMass / (m_Radius * m_Radius); //SOLID
	//float i = 1.5f * invMass / (m_Radius * m_Radius); //HOLLOW

	Matrix3 inertia;
	inertia._11 = i;
	inertia._22 = i;
	inertia._33 = i;

	return inertia;
}

void SphereCollisionShape::GetCollisionAxes(const PhysicsObject* currentObject, std::vector<Vector3>* out_axes) const
{
	/* There is infinite possible axes on a sphere so we MUST handle it seperately */
}

void SphereCollisionShape::GetEdges(const PhysicsObject* currentObject, std::vector<CollisionEdge>* out_edges) const
{
	/* There is infinite edges on a sphere so we MUST handle it seperately */
}

void SphereCollisionShape::GetMinMaxVertexOnAxis(const PhysicsObject* currentObject, const Vector3& axis, Vector3* out_min, Vector3* out_max) const
{
	if (out_min)
		*out_min = currentObject->GetPosition() - axis * m_Radius;

	if (out_max)
		*out_max = currentObject->GetPosition() + axis * m_Radius;
}

void SphereCollisionShape::GetIncidentReferencePolygon(const PhysicsObject* currentObject, const Vector3& axis, std::list<Vector3>* out_face, Vector3* out_normal, std::vector<Plane>* out_adjacent_planes) const
{
	if (out_face)
		out_face->push_back(currentObject->GetPosition() + axis * m_Radius);
	

	if (out_normal)
		*out_normal = axis;
	
}

void SphereCollisionShape::DebugDraw(const PhysicsObject* currentObject) const
{
	Vector3 pos = currentObject->GetPosition();

	//Draw Filled Circle
	NCLDebug::DrawPointNDT(pos, m_Radius, Vector4(1.0f, 1.0f, 1.0f, 0.2f));

	//Draw Perimeter Axes
	Vector3 lastX = pos + Vector3(0.0f, 1.0f, 0.0f) * m_Radius;
	Vector3 lastY = pos + Vector3(1.0f, 0.0f, 0.0f) * m_Radius;
	Vector3 lastZ = pos + Vector3(1.0f, 0.0f, 0.0f) * m_Radius;
	for (int itr = 1; itr <= 20; ++itr)
	{
		float angle = itr / 20.0f * 6.2831853f;
		float alpha = cosf(angle) * m_Radius;
		float beta = sinf(angle) * m_Radius;

		Vector3 newX = pos + Vector3(0.0f, alpha, beta);
		Vector3 newY = pos + Vector3(alpha, 0.0f, beta);
		Vector3 newZ = pos + Vector3(alpha, beta, 0.0f);

		NCLDebug::DrawThickLineNDT(lastX, newX, 0.02f, Vector4(1.0f, 0.3f, 1.0f, 1.0f));
		NCLDebug::DrawThickLineNDT(lastY, newY, 0.02f, Vector4(1.0f, 0.3f, 1.0f, 1.0f));
		NCLDebug::DrawThickLineNDT(lastZ, newZ, 0.02f, Vector4(1.0f, 0.3f, 1.0f, 1.0f));

		lastX = newX;
		lastY = newY;
		lastZ = newZ;
	}
}