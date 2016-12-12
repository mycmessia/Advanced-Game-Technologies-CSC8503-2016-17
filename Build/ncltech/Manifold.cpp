#include "Manifold.h"
#include <nclgl\Matrix3.h>
#include "NCLDebug.h"
#include "PhysicsEngine.h"

#define persistentThresholdSq 0.025f

typedef std::list<ContactPoint> ContactList;
typedef ContactList::iterator ContactListItr;

Manifold::Manifold() 
	: m_pNodeA(NULL)
	, m_pNodeB(NULL)
{
}

Manifold::~Manifold()
{

}

void Manifold::Initiate(PhysicsObject* nodeA, PhysicsObject* nodeB)
{
	m_vContacts.clear();

	m_pNodeA = nodeA;
	m_pNodeB = nodeB;
}

void Manifold::ApplyImpulse()
{
	/* TUT 6 CODE HERE */
	for (ContactPoint & contact : m_vContacts)
	{
		SolveContactPoint(contact);
	}
}


void Manifold::SolveContactPoint(ContactPoint& c)
{
	/* TUT 6 CODE HERE */
	if (m_pNodeA -> GetInverseMass() + m_pNodeB -> GetInverseMass() == 0.0f)
	return;
	
	Vector3 r1 = c.relPosA;
	Vector3 r2 = c.relPosB;
	
	Vector3 v0 = m_pNodeA -> GetLinearVelocity()
	+ Vector3::Cross(m_pNodeA -> GetAngularVelocity(), r1);
	Vector3 v1 = m_pNodeB -> GetLinearVelocity()
	+ Vector3::Cross(m_pNodeB -> GetAngularVelocity(), r2);
	
	Vector3 normal = c.collisionNormal;
	Vector3 dv = v0 - v1;

	// Collision Resolution
	{
		float constraintMass = (m_pNodeA -> GetInverseMass()
			+ m_pNodeB -> GetInverseMass())
			+ Vector3::Dot(normal,
			Vector3::Cross(m_pNodeA -> GetInverseInertia()
			* Vector3::Cross(r1, normal), r1)
			+ Vector3::Cross(m_pNodeB -> GetInverseInertia()
			* Vector3::Cross(r2, normal), r2));
		// Baumgarte Offset ( Adds energy to the system to counter
		// slight solving errors that accumulate over time
		// called as ’constraint drift ’)
		
		float b = 0.0f;
		{
			float distance_offset = c.collisionPenetration;
			float baumgarte_scalar = 0.3f; // Amount of force to
			// add to the system to solve error
			
			float baumgarte_slop = 0.001f; // Amount of a l l o w e d
			// penetration , ensures a complete manifold each frame
			
			float penetration_slop =
			min(c.collisionPenetration + baumgarte_slop, 0.0f);
			
			b = -(baumgarte_scalar
			/ PhysicsEngine::Instance() -> GetDeltaTime())
			* penetration_slop;
		}

		float b_real = max(b, c.elatisity_term + b * 0.2f);
		float jn = -(Vector3::Dot(dv, normal) + b_real) / constraintMass;

		//jn = min(jn, 0.0f);
		float oldSumImpulseContact = c.sumImpulseContact;
		c.sumImpulseContact = min(c.sumImpulseContact + jn, 0.0f);
		jn = c.sumImpulseContact - oldSumImpulseContact;

		m_pNodeA -> SetLinearVelocity(m_pNodeA -> GetLinearVelocity()
		+ normal * (jn * m_pNodeA -> GetInverseMass()));
		m_pNodeB -> SetLinearVelocity(m_pNodeB -> GetLinearVelocity()
		- normal * (jn * m_pNodeB -> GetInverseMass()));
		
		m_pNodeA -> SetAngularVelocity(m_pNodeA -> GetAngularVelocity()
		+ m_pNodeA -> GetInverseInertia()
		* Vector3::Cross(r1, normal * jn));
		m_pNodeB -> SetAngularVelocity(m_pNodeB -> GetAngularVelocity()
		- m_pNodeB -> GetInverseInertia()
		* Vector3::Cross(r2, normal * jn));
	}
	// Friction
	{
		Vector3 tangent = dv - normal * Vector3::Dot(dv, normal);
		float tangent_len = tangent.Length();
		
		if (tangent_len > 0.001f)
		{
			tangent = tangent * (1.0f / tangent_len);
			
			float frictionalMass =
			(m_pNodeA -> GetInverseMass()
			+ m_pNodeB -> GetInverseMass())
			+ Vector3::Dot(tangent,
			Vector3::Cross(m_pNodeA -> GetInverseInertia()
			* Vector3::Cross(r1, tangent), r1)
			+ Vector3::Cross(m_pNodeB -> GetInverseInertia()
			* Vector3::Cross(r2, tangent), r2));
			
			float frictionCoef = sqrtf(m_pNodeA -> GetFriction()
			* m_pNodeB -> GetFriction());
			
			float jt = -1 * frictionCoef * Vector3::Dot(dv, tangent)
			/ frictionalMass;
			
			// Clamp friction to never apply more force than the main collision
			// resolution force
			
			float oldImpulseTangent = c.sumImpulseFriction; //( < - - This should
			// be a float not a Vector3 )
			float maxJt = frictionCoef * c.sumImpulseContact;
			c.sumImpulseFriction =
			min(max(oldImpulseTangent + jt, maxJt), -maxJt);
			jt = c.sumImpulseFriction - oldImpulseTangent;

			m_pNodeA -> SetLinearVelocity(m_pNodeA -> GetLinearVelocity()
			+ tangent *(jt * m_pNodeA -> GetInverseMass()));
			m_pNodeB -> SetLinearVelocity(m_pNodeB -> GetLinearVelocity()
			 - tangent *(jt * m_pNodeB -> GetInverseMass()));
			
			m_pNodeA -> SetAngularVelocity(m_pNodeA -> GetAngularVelocity()
			+ m_pNodeA -> GetInverseInertia()
			* Vector3::Cross(r1, tangent * jt));
			m_pNodeB -> SetAngularVelocity(m_pNodeB -> GetAngularVelocity()
			- m_pNodeB -> GetInverseInertia()
			* Vector3::Cross(r2, tangent * jt));
		 }
	}
}

void Manifold::PreSolverStep(float dt)
{
	for (ContactPoint& contact : m_vContacts)
	{
		UpdateConstraint(contact);
	}
}

void Manifold::UpdateConstraint(ContactPoint& contact)
{
	//Reset total impulse forces computed this physics timestep 
	contact.sumImpulseContact = 0.0f;
	contact.sumImpulseFriction = 0.0f;


	/* TUT 6 CODE HERE */
	// Compute Elasticity Term - must be computed prior to solving
	// ANY constraints otherwise the objects velocities may have
	// already changed in a different constraint and the elasticity
	// force will no longer be correct .
	{
		const float elasticity = sqrtf(m_pNodeA->GetElasticity()
			* m_pNodeB->GetElasticity());

		float elatisity_term =
			elasticity * Vector3::Dot(contact.collisionNormal,
				m_pNodeA->GetLinearVelocity()
				+ Vector3::Cross(contact.relPosA,
					m_pNodeA->GetAngularVelocity())
				- m_pNodeB->GetLinearVelocity()
				- Vector3::Cross(contact.relPosB,
					m_pNodeB->GetAngularVelocity()));

		if (elatisity_term < 0.0f)
		{
			contact.elatisity_term = 0.0f;
		}
		else
		{
			// Elasticity slop here is used to make objects come to
			// rest quicker . It works out if the elastic term is less
			// than a given value (0.2 m/s here ) and if it is , then we
			// assume it is too small to see and should ignore the
			// elasticity calculation . Most noticable when you have a
			// stack of objects , without this they will jitter alot .

			const float elasticity_slop = 0.2f;

			if (elatisity_term < elasticity_slop)
				elatisity_term = 0.0f;

			contact.elatisity_term = elatisity_term;
		}
	}
}

void Manifold::AddContact(const Vector3& globalOnA, const Vector3& globalOnB, const Vector3& _normal, const float& _penetration)
{
	//Get relative offsets from each object centre of mass
	// Used to compute rotational velocity at the point of contact.
	Vector3 r1 = (globalOnA - m_pNodeA->GetPosition());
	Vector3 r2 = (globalOnB - m_pNodeB->GetPosition());

	//Create our new contact descriptor
	ContactPoint contact;
	contact.relPosA = r1;
	contact.relPosB = r2;
	contact.collisionNormal = _normal;
	contact.collisionPenetration = _penetration;


	//Check to see if we already contain a contact point almost in that location
	const float min_allowed_dist_sq = 0.2f * 0.2f;
	bool should_add = true;
	for (auto itr = m_vContacts.begin(); itr != m_vContacts.end(); )
	{
		Vector3 ab = itr->relPosA - contact.relPosA;
		float distsq = Vector3::Dot(ab, ab);


		//Choose the contact point with the largest penetration and therefore the largest collision response
		if (distsq < min_allowed_dist_sq)
		{
			if (itr->collisionPenetration > contact.collisionPenetration)
			{
				itr = m_vContacts.erase(itr);
				continue;
			}
			else
			{
				should_add = false;
			}
			
		}
		
		itr++;
	}


	
	if (should_add)
		m_vContacts.push_back(contact);
}

void Manifold::DebugDraw() const
{
	if (m_vContacts.size() > 0)
	{
		//Loop around all contact points and draw them all as a line-fan
		Vector3 globalOnA1 = m_pNodeA->GetPosition() + m_vContacts.back().relPosA;
		for (const ContactPoint& contact : m_vContacts)
		{
			Vector3 globalOnA2 = m_pNodeA->GetPosition() + contact.relPosA;
			Vector3 globalOnB = m_pNodeB->GetPosition() + contact.relPosB;

			//Draw line to form area given by all contact points
			NCLDebug::DrawThickLineNDT(globalOnA1, globalOnA2, 0.02f, Vector4(0.0f, 1.0f, 0.0f, 1.0f));

			//Draw descriptors for indivdual contact point
			NCLDebug::DrawPointNDT(globalOnA2, 0.05f, Vector4(0.0f, 0.5f, 0.0f, 1.0f));
			NCLDebug::DrawThickLineNDT(globalOnB, globalOnA2, 0.01f, Vector4(1.0f, 0.0f, 1.0f, 1.0f));

			globalOnA1 = globalOnA2;
		}
	}
}