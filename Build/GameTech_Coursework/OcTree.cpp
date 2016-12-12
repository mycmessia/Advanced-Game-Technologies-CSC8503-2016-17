#include "OcTree.h"

OcTree::OcTree (Vector3 pos, float size, std::vector<PhysicsObject*> &v)
{
	m_region = new AABB (pos, size);
	m_physicsObjects = v;
}

OcTree::~OcTree ()
{
	for (unsigned i = 0; i < m_physicsObjects.size (); i++)
	{
		delete m_physicsObjects[i];
	}

	for (int i = 0; i < 8; i++)
	{
		delete m_childNodes[i];
	}

	delete m_region;
}

void OcTree::BulidOcTree ()
{
	if (m_region->GetSize ().x < 8.f)
		return;

	if (m_physicsObjects.size () <= 1)
		return;

	Vector3 size = m_region->GetSize ();
	float radius = size.x / 2;	// every AABB is a cube
	Vector3 AABBPos = m_region->GetPosition ();
	Vector3 center = AABBPos + Vector3 (size.x / 2.0f, size.y / 2.0f, size.z / 2.0f);

	AABB* octant[8];
	octant[0] = new AABB (AABBPos, radius);
	octant[1] = new AABB (Vector3 (AABBPos.x, AABBPos.y, AABBPos.z + radius), radius);
	octant[2] = new AABB (Vector3 (AABBPos.x + radius, AABBPos.y, AABBPos.z), radius);
	octant[3] = new AABB (Vector3 (AABBPos.x + radius, AABBPos.y, AABBPos.z + radius), radius);

	octant[4] = new AABB (Vector3 (AABBPos.x, AABBPos.y + radius, AABBPos.z), radius);
	octant[5] = new AABB (Vector3 (AABBPos.x, AABBPos.y + radius, AABBPos.z + radius), radius);
	octant[6] = new AABB (Vector3 (AABBPos.x + radius, AABBPos.y + radius, AABBPos.z), radius);
	octant[7] = new AABB (Vector3 (AABBPos.x + radius, AABBPos.y + radius, AABBPos.z + radius), radius);

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
				if (octant[j]->Contains (m_physicsObjects[i]))
				{
					octantVectorArr[j].push_back (m_physicsObjects[i]);
					deleteVector.push_back (i);
				}
			}
		}
	}

	// Remove physical object which can be contained by subAABB from this OcTree
	//for (unsigned i = 0; i < deleteVector.size (); i++)
	//{
	//	m_physicsObjects.erase (m_physicsObjects.begin () + i);
	//}

	for (int i = 0; i < 8; i++)
	{
		if (octantVectorArr[i].size () > 0)
		{
			m_childNodes[i] = CreateNode (octant[i], octantVectorArr[i]);
			m_childNodes[i]->BulidOcTree ();
		}
		else
		{
			m_childNodes[i] = nullptr;
		}
	}
}

OcTree* OcTree::CreateNode (AABB* octant, std::vector<PhysicsObject*> &physicsObjectVector)
{
	if (physicsObjectVector.size () == 0)
		return nullptr;

	OcTree* tree = new OcTree (octant->GetPosition (), octant->GetSize ().x, physicsObjectVector);

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