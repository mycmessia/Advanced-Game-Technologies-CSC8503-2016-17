/******************************************************************************
Class: DistanceConstraint
Implements:
Author: Pieran Marris      <p.marris@newcastle.ac.uk> and YOU!
Description:

Manages a distance constraint between two objects, ensuring the two objects never
seperate. It works on a velocity level, enforcing the constraint:
	dot([(velocity of B) - (velocity of A)], normal) = zero

Thus ensuring that after integrating the position through the time, the distance between
the two objects never changes. 

		(\_/)
		( '_')
	 /""""""""""""\=========     -----D
	/"""""""""""""""""""""""\
....\_@____@____@____@____@_/

*//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Constraint.h"
#include "NCLDebug.h"
#include "PhysicsEngine.h"

class DistanceConstraint : public Constraint
{
public:
	DistanceConstraint(PhysicsObject* obj1, PhysicsObject* obj2,
		const Vector3& globalOnA, const Vector3& globalOnB)
	{
		m_pObj1 = obj1;
		m_pObj2 = obj2;

		Vector3 ab = globalOnB - globalOnA;
		m_Distance = ab.Length();

		Vector3 r1 = (globalOnA - m_pObj1->GetPosition());
		Vector3 r2 = (globalOnB - m_pObj2->GetPosition());
		m_LocalOnA = Matrix3::Transpose(m_pObj1->GetOrientation().ToMatrix3()) * r1;
		m_LocalOnB = Matrix3::Transpose(m_pObj2->GetOrientation().ToMatrix3()) * r2;
	}

	virtual void ApplyImpulse() override
	{
		/* TUT 3 */
		if (m_pObj1->GetInverseMass () + m_pObj2->GetInverseMass () == 0.0f)
			return;

		Vector3 r1 = m_pObj1->GetOrientation ().ToMatrix3 () * m_LocalOnA;
		Vector3 r2 = m_pObj2->GetOrientation ().ToMatrix3 () * m_LocalOnB;

		Vector3 globalOnA = r1 + m_pObj1->GetPosition ();
		Vector3 globalOnB = r2 + m_pObj2->GetPosition ();

		Vector3 ab = globalOnB - globalOnA;
		Vector3 abn = ab;
		abn.Normalise ();

		Vector3 v0 = m_pObj1->GetLinearVelocity () + Vector3::Cross (m_pObj1->GetAngularVelocity (), r1);
		Vector3 v1 = m_pObj2->GetLinearVelocity () + Vector3::Cross (m_pObj2->GetAngularVelocity (), r2);

		{
			float constraintMass = (m_pObj1->GetInverseMass () + m_pObj2->GetInverseMass ()) + 
				Vector3::Dot (abn, 
					Vector3::Cross (m_pObj1->GetInverseInertia () * Vector3::Cross (r1, abn), r1)
				+ Vector3::Cross (m_pObj2->GetInverseInertia () * Vector3::Cross (r2, abn), r2));

			float b = 0.0f;
			{
				float distance_offset = ab.Length () - m_Distance;
				float baumgarte_scalar = 0.1f;
				b = -(baumgarte_scalar / PhysicsEngine::Instance ()->GetDeltaTime ()) * distance_offset;
			}

			float jn = -(Vector3::Dot (v0 - v1, abn) + b) / constraintMass;

			m_pObj1->SetLinearVelocity (m_pObj1->GetLinearVelocity () + abn * (jn * m_pObj1->GetInverseMass ()));
			m_pObj2->SetLinearVelocity (m_pObj2->GetLinearVelocity () - abn * (jn * m_pObj2->GetInverseMass ()));

			m_pObj1->SetAngularVelocity (m_pObj1->GetAngularVelocity () + m_pObj1->GetInverseInertia () * Vector3::Cross (r1, abn * jn));
			m_pObj2->SetAngularVelocity (m_pObj2->GetAngularVelocity () - m_pObj2->GetInverseInertia () * Vector3::Cross (r2, abn * jn));
		}
	}

	virtual void DebugDraw() const
	{
		Vector3 globalOnA = m_pObj1->GetOrientation().ToMatrix3() * m_LocalOnA + m_pObj1->GetPosition();
		Vector3 globalOnB = m_pObj2->GetOrientation().ToMatrix3() * m_LocalOnB + m_pObj2->GetPosition();

		NCLDebug::DrawThickLine(globalOnA, globalOnB, 0.02f, Vector4(0.0f, 0.0f, 0.0f, 1.0f));
		NCLDebug::DrawPointNDT(globalOnA, 0.05f, Vector4(1.0f, 0.8f, 1.0f, 1.0f));
		NCLDebug::DrawPointNDT(globalOnB, 0.05f, Vector4(1.0f, 0.8f, 1.0f, 1.0f));
	}

protected:
	PhysicsObject *m_pObj1, *m_pObj2;
	float   m_Distance;
	Vector3 m_LocalOnA;
	Vector3 m_LocalOnB;
};