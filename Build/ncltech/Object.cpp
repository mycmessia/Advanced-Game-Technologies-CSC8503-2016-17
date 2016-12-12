#include "Object.h"
#include "PhysicsEngine.h"

Object::Object(const std::string& name)
	: m_pScene(NULL)
	, m_pParent(NULL)
	, m_Name(name)
	, m_Colour(1.0f, 1.0f, 1.0f, 1.0f)
	, m_BoundingRadius(1.0f)
	, m_FrustumCullFlags(NULL)
	, m_pPhysicsObject(NULL)
{
	m_LocalTransform.ToIdentity();
	m_WorldTransform.ToIdentity();
}

Object::~Object()
{
	if (m_pPhysicsObject != NULL)
	{
		PhysicsEngine::Instance()->RemovePhysicsObject(m_pPhysicsObject);
		delete m_pPhysicsObject;
		m_pPhysicsObject = NULL;
	}
}


void Object::CreatePhysicsNode()
{
	if (m_pPhysicsObject == NULL)
	{
		m_pPhysicsObject = new PhysicsObject();
		m_pPhysicsObject->SetAssociatedObject(this);
		PhysicsEngine::Instance()->AddPhysicsObject(m_pPhysicsObject);
	}
}

Object*	Object::FindGameObject(const std::string& name)
{
	//Has this object got the same name?
	if (GetName().compare(name) == 0)
	{
		return this;
	}

	//Recursively search ALL child objects and return the first one matching the given name
	for (auto child : m_vpChildren)
	{
		//Has the object in question got the same name?
		Object* cObj = child->FindGameObject(name);
		if (cObj != NULL)
		{
			return cObj;
		}
	}

	//Object not found with the given name
	return NULL;
}

void Object::AddChildObject(Object* child)
{
	m_vpChildren.push_back(child);
	child->m_pParent = this;
	child->m_pScene = this->m_pScene;
}
