#include "CommonUtils.h"

#include "ObjectMesh.h"
#include "ObjectMeshDragable.h"
#include "SphereCollisionShape.h"
#include "CuboidCollisionShape.h"
#include "CommonMeshes.h"

Vector4 CommonUtils::GenColour(float scalar, float alpha)
{
	Vector4 c;
	c.w = alpha;

	float t;
	c.x = abs(modf(scalar + 1.0f, &t) * 6.0f - 3.0f) - 1.0f;
	c.y = abs(modf(scalar + 2.0f / 3.0f, &t) * 6.0f - 3.0f) - 1.0f;
	c.z = abs(modf(scalar + 1.0f / 3.0f, &t) * 6.0f - 3.0f) - 1.0f;

	c.x = min(max(c.x, 0.0f), 1.0f);
	c.y = min(max(c.y, 0.0f), 1.0f);
	c.z = min(max(c.z, 0.0f), 1.0f);

	return c;
}

Object* CommonUtils::BuildSphereObject(
	const std::string& name,
	const Vector3& pos,
	float radius,
	bool physics_enabled,
	float inverse_mass,
	bool collidable,
	bool dragable,
	const Vector4& color)
{
	ObjectMesh* pSphere = dragable
		? new ObjectMeshDragable(name)
		: new ObjectMesh(name);

	pSphere->SetMesh(CommonMeshes::Sphere(), false);
	pSphere->SetTexture(CommonMeshes::CheckerboardTex(), false);
	pSphere->SetLocalTransform(Matrix4::Scale(Vector3(radius, radius, radius)));
	pSphere->SetColour(color);
	pSphere->SetBoundingRadius(radius);

	if (!physics_enabled)
	{
		//If no physics object is present, just set the local transform (modelMatrix) directly
		pSphere->SetLocalTransform(Matrix4::Translation(pos) * pSphere->GetLocalTransform());
	}
	else
	{
		//Otherwise create a physics object, and set it's position etc
		pSphere->CreatePhysicsNode();

		pSphere->Physics()->SetPosition(pos);
		pSphere->Physics()->SetInverseMass(inverse_mass);
		
		if (!collidable)
		{
			//Even without a collision shape, the inertia matrix for rotation has to be derived from the objects shape
			pSphere->Physics()->SetInverseInertia(SphereCollisionShape(radius).BuildInverseInertia(inverse_mass));
		}
		else
		{
			CollisionShape* pColshape = new SphereCollisionShape(radius);
			pSphere->Physics()->SetCollisionShape(pColshape);
			pSphere->Physics()->SetInverseInertia(pColshape->BuildInverseInertia(inverse_mass));
		}			
	}

	return pSphere;
}

Object* CommonUtils::BuildCuboidObject(
	const std::string& name,
	const Vector3& pos,
	const Vector3& halfdims,
	bool physics_enabled,
	float inverse_mass,
	bool collidable,
	bool dragable,
	const Vector4& color)
{
	ObjectMesh* pCuboid = dragable
		? new ObjectMeshDragable(name)
		: new ObjectMesh(name);

	pCuboid->SetMesh(CommonMeshes::Cube(), false);
	pCuboid->SetTexture(CommonMeshes::CheckerboardTex(), false);
	pCuboid->SetLocalTransform(Matrix4::Scale(halfdims));
	pCuboid->SetColour(color);
	pCuboid->SetBoundingRadius(halfdims.Length());

	if (!physics_enabled)
	{
		//If no physics object is present, just set the local transform (modelMatrix) directly
		pCuboid->SetLocalTransform(Matrix4::Translation(pos) * pCuboid->GetLocalTransform());
	}
	else
	{
		//Otherwise create a physics object, and set it's position etc
		pCuboid->CreatePhysicsNode();
		pCuboid->Physics()->SetPosition(pos);
		pCuboid->Physics()->SetInverseMass(inverse_mass);

		if (!collidable)
		{
			//Even without a collision shape, the inertia matrix for rotation has to be derived from the objects shape
			pCuboid->Physics()->SetInverseInertia(CuboidCollisionShape(halfdims).BuildInverseInertia(inverse_mass));
		}
		else
		{
			CollisionShape* pColshape = new CuboidCollisionShape(halfdims);
			pCuboid->Physics()->SetCollisionShape(pColshape);
			pCuboid->Physics()->SetInverseInertia(pColshape->BuildInverseInertia(inverse_mass));
		}
	}

	return pCuboid;
}

Object* CommonUtils::BuildTargetCuboidObject (
	const std::string& name,
	const Vector3& pos,
	const Vector3& halfdims,
	bool physics_enabled,
	float inverse_mass,
	bool collidable,
	bool dragable,
	const Vector4& color)
{
	ObjectMesh* pCuboid = dragable
	? new ObjectMeshDragable(name)
	: new ObjectMesh(name);

	pCuboid->SetMesh(CommonMeshes::Cube(), false);
	pCuboid->SetTexture (CommonMeshes::TargetTex (), false);
	pCuboid->SetLocalTransform(Matrix4::Scale(halfdims));
	pCuboid->SetColour(color);
	pCuboid->SetBoundingRadius(halfdims.Length());

	if (!physics_enabled)
	{
		//If no physics object is present, just set the local transform (modelMatrix) directly
		pCuboid->SetLocalTransform(Matrix4::Translation(pos) * pCuboid->GetLocalTransform());
	}
	else
	{
		//Otherwise create a physics object, and set it's position etc
		pCuboid->CreatePhysicsNode();
		pCuboid->Physics()->SetPosition(pos);
		pCuboid->Physics()->SetInverseMass(inverse_mass);

		if (!collidable)
		{
			//Even without a collision shape, the inertia matrix for rotation has to be derived from the objects shape
			pCuboid->Physics()->SetInverseInertia(CuboidCollisionShape(halfdims).BuildInverseInertia(inverse_mass));
		}
		else
		{
			CollisionShape* pColshape = new CuboidCollisionShape(halfdims);
			pCuboid->Physics()->SetCollisionShape(pColshape);
			pCuboid->Physics()->SetInverseInertia(pColshape->BuildInverseInertia(inverse_mass));
		}
	}

	return pCuboid;
}