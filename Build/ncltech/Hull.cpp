#include "Hull.h"
#include "NCLDebug.h"

Hull::Hull()
{

}

Hull::~Hull()
{

}


void Hull::AddVertex(const Vector3& v)
{
	HullVertex new_vertex;
	new_vertex.idx = m_vVertices.size();
	new_vertex.pos = v;

	m_vVertices.push_back(new_vertex);
}

int Hull::FindEdge(int v0_idx, int v1_idx)
{
	for (const HullEdge& edge : m_vEdges)
	{
		if ((edge.vStart == v0_idx && edge.vEnd == v1_idx)
			|| (edge.vStart == v1_idx && edge.vEnd == v0_idx))
		{
			return edge.idx;
		}
	}

	return -1; //Not Found
}

int Hull::ConstructNewEdge(int parent_face_idx, int vert_start, int vert_end)
{
	int out_idx = FindEdge(vert_start, vert_end);


	//Edge not already within the Hull, 
	if (out_idx == -1)
	{
		out_idx = m_vEdges.size();

		HullEdge new_edge;
		new_edge.idx = m_vEdges.size();
		new_edge.vStart = vert_start;
		new_edge.vEnd = vert_end;
		m_vEdges.push_back(new_edge);

		HullEdge* new_edge_ptr = &m_vEdges[new_edge.idx];

		//Find Adjacent Edges
		for (int i = 0; i < new_edge.idx; ++i)
		{
			if (m_vEdges[i].vStart == vert_start
				|| m_vEdges[i].vStart == vert_end
				|| m_vEdges[i].vEnd == vert_start
				|| m_vEdges[i].vEnd == vert_end)
			{
				m_vEdges[i].adjoining_edge_ids.push_back(new_edge.idx);
				new_edge_ptr->adjoining_edge_ids.push_back(i);
			}
		}

		//Update Contained Vertices
		m_vVertices[vert_start].enclosing_edges.push_back(new_edge.idx);
		m_vVertices[vert_end].enclosing_edges.push_back(new_edge.idx);
	}

	m_vEdges[out_idx].enclosing_faces.push_back(parent_face_idx);
	return out_idx;
}

void Hull::AddFace(const Vector3& _normal, int nVerts, const int* verts)
{
	HullFace new_face;
	new_face.idx = m_vFaces.size();
	new_face._normal = _normal;
	new_face._normal.Normalise();

	m_vFaces.push_back(new_face);
	HullFace* new_face_ptr = &m_vFaces[new_face.idx];



	//Construct all contained edges
	int p0 = nVerts - 1;
	for (int p1 = 0; p1 < nVerts; ++p1)
	{
		new_face_ptr->vert_ids.push_back(verts[p1]);
		new_face_ptr->edge_ids.push_back(ConstructNewEdge(new_face.idx, verts[p0], verts[p1]));
		p0 = p1;
	}


	//Find Adjacent Faces
	for (int i = 0; i < new_face.idx; ++i)
	{
		HullFace& cFace = m_vFaces[i];
		bool found = false;
		for (size_t j = 0; found == false && j < cFace.edge_ids.size(); ++j)
		{
			for (int k = 0; found == false && k < nVerts; ++k)
			{
				if (new_face_ptr->edge_ids[k] == cFace.edge_ids[j])
				{
					found = true;
					cFace.adjoining_face_ids.push_back(new_face.idx);
					new_face_ptr->adjoining_face_ids.push_back(i);
				}
			}
		}
	}

	//Update Contained Vertices
	for (int i = 0; i < nVerts; ++i)
	{
		HullVertex* cVertStart	= &m_vVertices[m_vEdges[new_face_ptr->edge_ids[i]].vStart];
		HullVertex* cVertEnd	= &m_vVertices[m_vEdges[new_face_ptr->edge_ids[i]].vEnd];

		auto foundLocStart = std::find(cVertStart->enclosing_faces.begin(), cVertStart->enclosing_faces.end(), new_face.idx);
		if (foundLocStart == cVertStart->enclosing_faces.end())
		{
			cVertStart->enclosing_faces.push_back(new_face.idx);
		}

		auto foundLocEnd = std::find(cVertEnd->enclosing_faces.begin(), cVertEnd->enclosing_faces.end(), new_face.idx);
		if (foundLocEnd == cVertEnd->enclosing_faces.end())
		{
			cVertEnd->enclosing_faces.push_back(new_face.idx);
		}
	}
}


void Hull::GetMinMaxVerticesInAxis(const Vector3& local_axis, int* out_min_vert, int* out_max_vert)
{
	float cCorrelation;
	int minVertex, maxVertex;

	float minCorrelation = FLT_MAX, maxCorrelation = -FLT_MAX;

	for (size_t i = 0; i < m_vVertices.size(); ++i)
	{
		cCorrelation = Vector3::Dot(local_axis, m_vVertices[i].pos);

		if (cCorrelation > maxCorrelation)
		{
			maxCorrelation = cCorrelation;
			maxVertex = i;
		}

		if (cCorrelation <= minCorrelation)
		{
			minCorrelation = cCorrelation;
			minVertex = i;
		}
	}

	if (out_min_vert) *out_min_vert = minVertex;
	if (out_max_vert) *out_max_vert = maxVertex;
}


void Hull::DebugDraw(const Matrix4& transform)
{
	//Draw all Hull Polygons
	for (HullFace& face : m_vFaces)
	{
		//Render Polygon as triangle fan
		if (face.vert_ids.size() > 2)
		{
			Vector3 polygon_start = transform * m_vVertices[face.vert_ids[0]].pos;
			Vector3 polygon_last = transform * m_vVertices[face.vert_ids[1]].pos;

			for (size_t idx = 2; idx < face.vert_ids.size(); ++idx)
			{
				Vector3 polygon_next = transform * m_vVertices[face.vert_ids[idx]].pos;

				NCLDebug::DrawTriangleNDT(polygon_start, polygon_last, polygon_next, Vector4(1.0f, 1.0f, 1.0f, 0.2f));
				polygon_last = polygon_next;
			}
		}
	}

	//Draw all Hull Edges
	for (HullEdge& edge : m_vEdges)
	{
		NCLDebug::DrawThickLineNDT(transform * m_vVertices[edge.vStart].pos, transform * m_vVertices[edge.vEnd].pos, 0.02f, Vector4(1.0f, 0.2f, 1.0f, 1.0f));
	}
}