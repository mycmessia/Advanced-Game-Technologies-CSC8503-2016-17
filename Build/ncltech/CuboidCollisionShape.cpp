#include "CuboidCollisionShape.h"
#include "PhysicsObject.h"
#include <nclgl/Matrix3.h>
#include <nclgl/OGLRenderer.h>

Hull CuboidCollisionShape::m_CubeHull = Hull();

CuboidCollisionShape::CuboidCollisionShape()
{
	m_CuboidHalfDimensions = Vector3(0.5f, 0.5f, 0.5f);

	if (m_CubeHull.GetNumVertices() == 0)
	{
		ConstructCubeHull();
	}
}

CuboidCollisionShape::CuboidCollisionShape(const Vector3& halfdims)
{
	m_CuboidHalfDimensions = halfdims;

	if (m_CubeHull.GetNumVertices() == 0)
	{
		ConstructCubeHull();
	}
}

CuboidCollisionShape::~CuboidCollisionShape()
{

}

Matrix3 CuboidCollisionShape::BuildInverseInertia(float invMass) const
{
	Matrix3 inertia;

	Vector3 dimsSq = (m_CuboidHalfDimensions + m_CuboidHalfDimensions);
	dimsSq = dimsSq * dimsSq;

	inertia._11 = 12.f * invMass / (dimsSq.y + dimsSq.z);
	inertia._22 = 12.f * invMass / (dimsSq.x + dimsSq.z);
	inertia._33 = 12.f * invMass / (dimsSq.x + dimsSq.y);
	
	return inertia;
}

void CuboidCollisionShape::GetCollisionAxes(const PhysicsObject* currentObject, std::vector<Vector3>* out_axes) const
{
	if (out_axes)
	{
		Matrix3 objOrientation = currentObject->GetOrientation().ToMatrix3();
		out_axes->push_back(objOrientation * Vector3(1.0f, 0.0f, 0.0f)); //X - Axis
		out_axes->push_back(objOrientation * Vector3(0.0f, 1.0f, 0.0f)); //Y - Axis
		out_axes->push_back(objOrientation * Vector3(0.0f, 0.0f, 1.0f)); //Z - Axis
	}
}

void CuboidCollisionShape::GetEdges(const PhysicsObject* currentObject, std::vector<CollisionEdge>* out_edges) const
{
	if (out_edges)
	{
		Matrix4 transform = currentObject->GetWorldSpaceTransform() * Matrix4::Scale(Vector3(m_CuboidHalfDimensions));
		for (unsigned int i = 0; i < m_CubeHull.GetNumEdges(); ++i)
		{
			const HullEdge& edge = m_CubeHull.GetEdge(i);
			Vector3 A = transform * m_CubeHull.GetVertex(edge.vStart).pos;
			Vector3 B = transform * m_CubeHull.GetVertex(edge.vEnd).pos;

			out_edges->push_back(CollisionEdge(A, B));
		}
	}
}

void CuboidCollisionShape::GetMinMaxVertexOnAxis(
	const PhysicsObject* currentObject,
	const Vector3& axis,
	Vector3* out_min,
	Vector3* out_max) const
{
	// Build World Transform
	Matrix4 wsTransform = currentObject->GetWorldSpaceTransform() * Matrix4::Scale(m_CuboidHalfDimensions);

	// Convert world space axis into model space (Axis Aligned Cuboid)
	Matrix3 invNormalMatrix = Matrix3::Transpose(Matrix3(wsTransform));
	Vector3 local_axis = invNormalMatrix * axis;
	local_axis.Normalise();

	// Get closest and furthest vertex id's
	int vMin, vMax;
	m_CubeHull.GetMinMaxVerticesInAxis(local_axis, &vMin, &vMax);

	// Return closest and furthest vertices in world-space
	if (out_min) *out_min = wsTransform * m_CubeHull.GetVertex(vMin).pos;
	if (out_max) *out_max = wsTransform * m_CubeHull.GetVertex(vMax).pos;
}



void CuboidCollisionShape::GetIncidentReferencePolygon(
	const PhysicsObject* currentObject,
	const Vector3& axis,
	std::list<Vector3>* out_face,
	Vector3* out_normal,
	std::vector<Plane>* out_adjacent_planes) const
{
	//Get the world-space transform
	Matrix4 wsTransform = currentObject->GetWorldSpaceTransform() * Matrix4::Scale(m_CuboidHalfDimensions);

	//Get normal and inverse-normal matrices to transfom the collision axis to and from modelspace
	Matrix3 invNormalMatrix = Matrix3::Transpose(Matrix3(wsTransform));
	Matrix3 normalMatrix = Matrix3::Inverse(invNormalMatrix);
	

	Vector3 local_axis = invNormalMatrix * axis;


	//Get the furthest vertex along axis - this will be part of the further face
	int undefined, maxVertex;
	m_CubeHull.GetMinMaxVerticesInAxis(local_axis, &undefined, &maxVertex);
	const HullVertex& vert = m_CubeHull.GetVertex(maxVertex);


	//Compute which face (that contains the furthest vertex above)
	// is the furthest along the given axis. This is defined by
	// it's normal being closest to parallel with the collision axis.
	const HullFace* best_face = 0;
	float best_correlation = -FLT_MAX;
	for (int faceIdx : vert.enclosing_faces)
	{
		const HullFace* face = &m_CubeHull.GetFace(faceIdx);
		float temp_correlation = Vector3::Dot(local_axis, face->_normal);
		if (temp_correlation > best_correlation)
		{
			best_correlation = temp_correlation;
			best_face = face;
		}
	}


	// Output face normal
	if (out_normal)
	{
		*out_normal = normalMatrix * best_face->_normal;
		(*out_normal).Normalise();
	}

	// Output face vertices (transformed back into world-space)
	if (out_face)
	{
		for (int vertIdx : best_face->vert_ids)
		{
			const HullVertex& vert = m_CubeHull.GetVertex(vertIdx);
			out_face->push_back(wsTransform * vert.pos);
		}
	}

	
	// Now, we need to define a set of planes that will clip any 3d geometry down to fit inside 
	// the shape. This results in us forming list of clip planes from each of the
	// adjacent faces along with the reference face itself.
	if (out_adjacent_planes)
	{
		Vector3 wsPointOnPlane = wsTransform * m_CubeHull.GetVertex(m_CubeHull.GetEdge(best_face->edge_ids[0]).vStart).pos;

		// First, form a plane around the reference face
		{
			//We use the negated normal here for the plane, as we want to clip geometry left outside the shape not inside it.
			Vector3 planeNrml = -(normalMatrix * best_face->_normal);	
			planeNrml.Normalise();

			float planeDist = -Vector3::Dot(planeNrml, wsPointOnPlane);
			out_adjacent_planes->push_back(Plane(planeNrml, planeDist));
		}
		
		// Now we need to loop over all adjacent faces, and form a similar
		// clip plane around those too.
		// - The way that the HULL object is constructed means each edge can only
		//   ever have two adjoining faces. This means we can iterate through all
		//   edges of the face and then build a plane around the other face that
		//   also shares that edge.
		for (int edgeIdx : best_face->edge_ids)
		{
			const HullEdge& edge = m_CubeHull.GetEdge(edgeIdx);

			wsPointOnPlane = wsTransform * m_CubeHull.GetVertex(edge.vStart).pos;

			for (int adjFaceIdx : edge.enclosing_faces)
			{
				if (adjFaceIdx != best_face->idx)
				{
					const HullFace& adjFace = m_CubeHull.GetFace(adjFaceIdx);

					Vector3 planeNrml = -(normalMatrix * adjFace._normal);
					planeNrml.Normalise();
					float planeDist = -Vector3::Dot(planeNrml, wsPointOnPlane);

					out_adjacent_planes->push_back(Plane(planeNrml, planeDist));
				}
			}	
		}
	}
}


void CuboidCollisionShape::DebugDraw(const PhysicsObject* currentObject) const
{
	// Just draw the cuboid hull-mesh at the position of our PhysicsObject
	Matrix4 transform = currentObject->GetWorldSpaceTransform() * Matrix4::Scale(m_CuboidHalfDimensions);
	m_CubeHull.DebugDraw(transform);
}

void CuboidCollisionShape::ConstructCubeHull()
{
	//Vertices
	m_CubeHull.AddVertex(Vector3(-1.0f, -1.0f, -1.0f));		// 0
	m_CubeHull.AddVertex(Vector3(-1.0f,  1.0f, -1.0f));		// 1
	m_CubeHull.AddVertex(Vector3( 1.0f,  1.0f, -1.0f));		// 2
	m_CubeHull.AddVertex(Vector3( 1.0f, -1.0f, -1.0f));		// 3

	m_CubeHull.AddVertex(Vector3(-1.0f, -1.0f,  1.0f));		// 4
	m_CubeHull.AddVertex(Vector3(-1.0f,  1.0f,  1.0f));		// 5
	m_CubeHull.AddVertex(Vector3( 1.0f,  1.0f,  1.0f));		// 6
	m_CubeHull.AddVertex(Vector3( 1.0f, -1.0f,  1.0f));		// 7

	//Indices ( MUST be provided in ccw winding order )
	int face1[] = { 0, 1, 2, 3 };
	int face2[] = { 7, 6, 5, 4 };
	int face3[] = { 5, 6, 2, 1 };
	int face4[] = { 0, 3, 7, 4 };
	int face5[] = { 6, 7, 3, 2 };
	int face6[] = { 4, 5, 1, 0 };

	//Faces
	m_CubeHull.AddFace(Vector3(0.0f, 0.0f, -1.0f), 4, face1);
	m_CubeHull.AddFace(Vector3(0.0f, 0.0f, 1.0f), 4, face2);
	m_CubeHull.AddFace(Vector3(0.0f, 1.0f, 0.0f), 4, face3);
	m_CubeHull.AddFace(Vector3(0.0f, -1.0f, 0.0f), 4, face4);
	m_CubeHull.AddFace(Vector3(1.0f, 0.0f, 0.0f), 4, face5);
	m_CubeHull.AddFace(Vector3(-1.0f, 0.0f, 0.0f), 4, face6);
}