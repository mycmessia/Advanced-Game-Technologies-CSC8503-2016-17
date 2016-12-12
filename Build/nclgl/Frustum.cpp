#include "Frustum.h"

void Frustum::FromMatrix(const Matrix4 &mat) {
	Vector3 xaxis = Vector3(mat.values[0],mat.values[4],mat.values[8]);
	Vector3 yaxis = Vector3(mat.values[1],mat.values[5],mat.values[9]);
	Vector3 zaxis = Vector3(mat.values[2],mat.values[6],mat.values[10]);
	Vector3 waxis = Vector3(mat.values[3],mat.values[7],mat.values[11]);

	//RIGHT
	planes[0] = Plane(waxis - xaxis, (mat.values[15] - mat.values[12]), true);

	//LEFT
	planes[1] = Plane(waxis + xaxis, (mat.values[15] + mat.values[12]), true);

	//BOTTOM
	planes[2] = Plane(waxis + yaxis, (mat.values[15] + mat.values[13]), true);

	//TOP
	planes[3] = Plane(waxis - yaxis, (mat.values[15] - mat.values[13]), true);

	//FAR
	planes[4] = Plane(waxis - zaxis, (mat.values[15] - mat.values[14]), true);

	//NEAR
	planes[5] = Plane(waxis + zaxis, (mat.values[15] + mat.values[14]), true);

	auto mtx_idx = [&](int col, int row)
	{
		return col * 4 + row;
	};

	auto build_plane = [&](int row, int sign)
	{
		return Plane(
			Vector3(
				mat[mtx_idx(0, 3)] + sign * mat[mtx_idx(0, row)],
				mat[mtx_idx(1, 3)] + sign * mat[mtx_idx(1, row)],
				mat[mtx_idx(2, 3)] + sign * mat[mtx_idx(2, row)]
			),
			mat[mtx_idx(3, 3)] + sign * mat[mtx_idx(3, row)],
			true);

	};

	planes[0] = build_plane(0, 1);
	planes[1] = build_plane(0, -1);
	planes[2] = build_plane(1, 1);
	planes[3] = build_plane(1, -1);
	planes[4] = build_plane(2, 1);
	planes[5] = build_plane(2, -1);

}

bool Frustum::InsideFrustum(const Vector3& position, float radius) const {
	for (int p = 0; p < 6; p++)	{
		if (!planes[p].SphereInPlane(position, radius)) {
			return false;
		}
	}
	return true;
}

bool Frustum::InsideFrustum(SceneNode&n) const	{
	for(int p = 0; p < 6; p++ )	{
		//if(!planes[p].PointInPlane(n.GetWorldTransform().GetPositionVector())) {
		if(!planes[p].SphereInPlane(n.GetWorldTransform().GetPositionVector(),n.GetBoundingRadius())) {
			return false;
		}
	}
	return true;
}

bool	Frustum::AABBInsideFrustum(Vector3 &position, const Vector3 &size)	const {
	for(int p = 0; p < 6; p++ )	{
		if(planes[p].PointInPlane(Vector3(position.x -size.x , position.y +size.y,position.z +size.z))) {
			continue;
		}
		if(planes[p].PointInPlane(Vector3(position.x +size.x , position.y +size.y,position.z +size.z))) {
			continue;
		}
		if(planes[p].PointInPlane(Vector3(position.x -size.x , position.y -size.y,position.z +size.z))) {
			continue;
		}
		if(planes[p].PointInPlane(Vector3(position.x +size.x , position.y -size.y,position.z +size.z))) {
			continue;
		}

		if(planes[p].PointInPlane(Vector3(position.x -size.x , position.y +size.y,position.z -size.z))) {
			continue;
		}
		if(planes[p].PointInPlane(Vector3(position.x +size.x , position.y +size.y,position.z -size.z))) {
			continue;
		}
		if(planes[p].PointInPlane(Vector3(position.x -size.x , position.y -size.y,position.z -size.z))) {
			continue;
		}
		if(planes[p].PointInPlane(Vector3(position.x +size.x , position.y -size.y,position.z -size.z))) {
			continue;
		}

		return false;
	}

	return true;
}