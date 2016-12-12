#include "PhysicsObject.h"
#include "PhysicsEngine.h"

PhysicsObject::PhysicsObject()
	: m_wsTransformInvalidated(true)
	, m_Enabled(false)
	, m_Position(0.0f, 0.0f, 0.0f)
	, m_LinearVelocity(0.0f, 0.0f, 0.0f)
	, m_Force(0.0f, 0.0f, 0.0f)
	, m_InvMass(0.0f)
	, m_Orientation(0.0f, 0.0f, 0.0f, 1.0f)
	, m_AngularVelocity(0.0f, 0.0f, 0.0f)
	, m_Torque(0.0f, 0.0f, 0.0f)
	, m_InvInertia(Matrix3::ZeroMatrix)
	, m_pColShape(NULL)
	, m_Friction(0.5f)
	, m_Elasticity(0.9f)
	, m_OnCollisionCallback(nullptr)
	, m_isColl(false)
	, m_isInAtmosphere (false)
	, m_isBullet (false)
	, m_isTarget (false)
	, m_isHitTarget (false)
	, m_isSleep (false)
{
}

PhysicsObject::~PhysicsObject()
{
	//Delete Collision Shape
	if (m_pColShape != NULL)
	{
		delete m_pColShape;
		m_pColShape = NULL;
	}
}

const Matrix4& PhysicsObject::GetWorldSpaceTransform() const 
{
	if (m_wsTransformInvalidated)
	{
		m_wsTransform = m_Orientation.ToMatrix4();
		m_wsTransform.SetPositionVector(m_Position);

		m_wsTransformInvalidated = false;
	}

	return m_wsTransform;
}