#include "AABB.h"
#include "OcTree.h"
#include "PhysicsObject.h"
#include "PhysicsEngine.h"

OcTree::OcTree (Vector3 pos, float size, std::vector<PhysicsObject*> &v)
{
	m_region = new AABB (pos, size);

	m_parent = nullptr;

	for (unsigned i = 0; i < v.size (); i++)
		m_physicsObjects.push_back (v[i]);
}

OcTree::~OcTree ()
{
	//Delete ();
}

void OcTree::BulidOcTree ()
{
	if (m_physicsObjects.size () <= 1)
		return;

	if (m_region->GetSize ().x < 2.f)
		return;

	Vector3 size = m_region->GetSize ();
	float radius = size.x / 2;	// every AABB is a cube
	Vector3 AABBPos = m_region->GetPosition ();
	Vector3 center = AABBPos + Vector3 (size.x / 2.0f, size.y / 2.0f, size.z / 2.0f);

	// Divide the current region to subAABBs
	AABB octant[8] {
		AABB (AABBPos, radius),
		AABB (Vector3 (AABBPos.x, AABBPos.y, AABBPos.z + radius), radius),
		AABB (Vector3 (AABBPos.x + radius, AABBPos.y, AABBPos.z), radius),
		AABB (Vector3 (AABBPos.x + radius, AABBPos.y, AABBPos.z + radius), radius),

		AABB (Vector3 (AABBPos.x, AABBPos.y + radius, AABBPos.z), radius),
		AABB (Vector3 (AABBPos.x, AABBPos.y + radius, AABBPos.z + radius), radius),
		AABB (Vector3 (AABBPos.x + radius, AABBPos.y + radius, AABBPos.z), radius),
		AABB (Vector3 (AABBPos.x + radius, AABBPos.y + radius, AABBPos.z + radius), radius)
	};

	std::vector< std::vector <PhysicsObject*> > octantVectorArr;

	for (int i = 0; i < 8; i++)
		octantVectorArr.push_back (std::vector <PhysicsObject*> ());

	std::vector<int> deleteVector;

	// Check if some physics objects can be contained by subAABB
	for (unsigned i = 0; i < m_physicsObjects.size (); i++)
	{
		if (radius > 0.0f)
		{
			for (int j = 0; j < 8; j++)
			{
				if (octant[j].Contains (m_physicsObjects[i]))
				{
					octantVectorArr[j].push_back (m_physicsObjects[i]);
					deleteVector.push_back (i);
				}
			}
		}
	}

	// Remove physical object which can be contained by subAABB from this OcTree
	for (unsigned i = 0; i < deleteVector.size (); i++)
	{
		m_physicsObjects.erase (m_physicsObjects.begin () + (deleteVector[i] - i));
	}

	for (int i = 0; i < 8; i++)
	{
		if (octantVectorArr[i].size () > 0)
		{
			m_childNodes[i] = CreateNode (octant[i], octantVectorArr[i]);
			m_childNodes[i]->m_parent = this;
			m_childNodes[i]->BulidOcTree ();
		}
		else
		{
			m_childNodes[i] = nullptr;
		}
	}
}

OcTree* OcTree::CreateNode (AABB octant, std::vector<PhysicsObject*> &physicsObjectVector)
{
	if (physicsObjectVector.size () == 0)
		return nullptr;

	OcTree* tree = new OcTree (octant.GetPosition (), octant.GetSize ().x, physicsObjectVector);

	return tree;
}

void OcTree::Draw ()
{
	for (int i = 0; i < 8; i++)
	{
		if (m_childNodes[i])
		{
			m_childNodes[i]->Draw ();
		}
	}

	m_region->Draw ();
}

void OcTree::Delete ()
{
	for (int i = 0; i < 8; i++)
	{
		if (m_childNodes[i])
		{
			m_childNodes[i]->Delete ();
			delete m_childNodes[i];
			m_childNodes[i] = NULL;
		}
	}

	//for (unsigned i = 0; i < m_physicsObjects.size (); i++)
	//{
	//	delete m_physicsObjects[i];
	//}

	// just clean the vector do not need to delete m_physicsObjects[i]!!!! 
	m_physicsObjects.clear();

	//if (m_region)
	//{
		delete m_region;
		m_region = NULL;
	//}
}

void OcTree::GenerateCPs (std::vector<CollisionPair> &cpList)
{
	// GenerateCPs recursively
	for (int i = 0; i < 8; i++)
	{
		if (m_childNodes[i])
		{
			m_childNodes[i]->GenerateCPs (cpList);
		}
	}

	PhysicsObject *m_pObj1, *m_pObj2;

	OcTree* parent = m_parent;
	while (parent)
	{
		for (unsigned i = 0; i < m_physicsObjects.size (); i++)
		{
			// Check collision with parent's physicsObjects
			for (unsigned j = 0; j < parent->m_physicsObjects.size(); ++j)
			{
				m_pObj1 = m_physicsObjects[i];
				m_pObj2 = parent->m_physicsObjects[j];

				if (m_pObj1->GetCollisionShape() != NULL
					&& m_pObj2->GetCollisionShape() != NULL)
				{
					CollisionPair cp;
					cp.pObjectA = m_pObj1;
					cp.pObjectB = m_pObj2;
					cpList.push_back(cp);
				}
			}
		}

		parent = parent->m_parent;
	}


	for (unsigned i = 0; i < m_physicsObjects.size (); i++)
	{
		for (unsigned j = i + 1; j < m_physicsObjects.size(); ++j)
		{
			m_pObj1 = m_physicsObjects[i];
			m_pObj2 = m_physicsObjects[j];

			//Check they both atleast have collision shapes
			if (m_pObj1->GetCollisionShape() != NULL
				&& m_pObj2->GetCollisionShape() != NULL)
			{
				CollisionPair cp;
				cp.pObjectA = m_pObj1;
				cp.pObjectB = m_pObj2;
				cpList.push_back(cp);
			}
		}
	}
}