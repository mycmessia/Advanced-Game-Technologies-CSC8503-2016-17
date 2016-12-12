/******************************************************************************
Class: Hull
Implements:
Author: Pieran Marris      <p.marris@newcastle.ac.uk> and YOU!
Description: 

This is an elaborate and /slow/ version of the Mesh class from Graphics for Games.
It keeps track of faces, vertices and edges of a given mesh aswell as all the adjancy
information.

This means that you can retrieve a face and instanty have a list of all of it's 
adjancent faces and contained vertices/edges without having to do expensive lookups.
Yep.. the expensive part is that these are all precomputed when the hull is first created O(n^3).

They can be quite useful for debugging shapes and experimenting with new 3D algorithms. 
In this framework they are used to represent discrete collision shapes which have distinct non-curved, 
faces such as the CuboidCollisionShape.

Note: One of the big changes from a normal Mesh is that the faces can have any number of vertices,
so you could represent your mesh as a series of pentagons, quads etc or any combination of these. 

		(\_/)
		( '_')
	 /""""""""""""\=========     -----D
	/"""""""""""""""""""""""\
....\_@____@____@____@____@_/

*//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <nclgl\Vector3.h>
#include <nclgl\Matrix4.h>
#include <vector>

struct HullEdge;
struct HullFace;

struct HullVertex
{
	int idx;
	Vector3 pos;
	std::vector<int> enclosing_edges;
	std::vector<int> enclosing_faces;
};

struct HullEdge
{
	int idx;
	int vStart, vEnd;
	std::vector<int> adjoining_edge_ids;
	std::vector<int> enclosing_faces;
};

struct HullFace
{
	int idx;
	Vector3 _normal;
	std::vector<int> vert_ids;
	std::vector<int> edge_ids;
	std::vector<int> adjoining_face_ids;
};

class Hull
{
public:
	Hull();
	~Hull();

	void AddVertex(const Vector3& v);
	

	void AddFace(const Vector3& _normal, int nVerts, const int* verts);
	void AddFace(const Vector3& _normal, const std::vector<int>& vert_ids)		{ AddFace(_normal, vert_ids.size(), &vert_ids[0]); }


	int FindEdge(int v0_idx, int v1_idx);
	

	const HullVertex& GetVertex(int idx)		{ return m_vVertices[idx]; }
	const HullEdge& GetEdge(int idx)			{ return m_vEdges[idx]; }
	const HullFace& GetFace(int idx)			{ return m_vFaces[idx]; }

	size_t GetNumVertices()					{ return m_vVertices.size(); }
	size_t GetNumEdges()					{ return m_vEdges.size(); }
	size_t GetNumFaces()					{ return m_vFaces.size(); }


	void GetMinMaxVerticesInAxis(const Vector3& local_axis, int* out_min_vert, int* out_max_vert);


	void DebugDraw(const Matrix4& transform);

protected:
	int ConstructNewEdge(int parent_face_idx, int vert_start, int vert_end); //Called by AddFace
	
protected:
	std::vector<HullVertex>		m_vVertices;
	std::vector<HullEdge>		m_vEdges;
	std::vector<HullFace>		m_vFaces;

};