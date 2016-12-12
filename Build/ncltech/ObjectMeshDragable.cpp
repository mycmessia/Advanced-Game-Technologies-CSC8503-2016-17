#include "ObjectMeshDragable.h"
#include "ScreenPicker.h"
#include "NCLDebug.h"

ObjectMeshDragable::ObjectMeshDragable(const std::string& name)
	: ObjectMesh(name)
	, m_LocalClickOffset(0.0f, 0.0f, 0.0f)
	, m_MouseDownColOffset(0.2f, 0.2f, 0.2f, 0.0f)
	, m_MouseOverColOffset(0.2f, 0.2f, 0.2f, 0.0f)
{
	//Register the object to listen for click callbacks
	ScreenPicker::Instance()->RegisterObject(this);
}

ObjectMeshDragable::~ObjectMeshDragable()
{
	//Unregister the object to prevent sending click events to undefined memory
	ScreenPicker::Instance()->UnregisterObject(this);
}

void ObjectMeshDragable::SetMouseOverColourOffset(const Vector4& col_offset)
{
	m_MouseOverColOffset = col_offset;
}

void ObjectMeshDragable::SetMouseDownColourOffset(const Vector4& col_offset)
{
	m_MouseOverColOffset = m_MouseDownColOffset - col_offset;
}

void ObjectMeshDragable::OnMouseEnter(float dt)
{
	this->m_Colour += m_MouseOverColOffset;
}

void ObjectMeshDragable::OnMouseLeave(float dt)
{
	this->m_Colour -= m_MouseOverColOffset;
}

void ObjectMeshDragable::OnMouseDown(float dt, const Vector3& worldPos)
{
	m_LocalClickOffset = worldPos - this->m_WorldTransform.GetPositionVector();
	this->m_Colour += m_MouseDownColOffset;
	
	if (this->HasPhysics())
	{
		this->Physics()->SetAngularVelocity(Vector3(0.0f, 0.0f, 0.0f));
		this->Physics()->SetLinearVelocity(Vector3(0.0f, 0.0f, 0.0f));
	}
}

void ObjectMeshDragable::OnMouseMove(float dt, const Vector3& worldPos, const Vector3& worldChange)
{
	Vector3 newpos = worldPos - m_LocalClickOffset;

	if (this->HasPhysics())
	{
		this->Physics()->SetPosition(worldPos - m_LocalClickOffset);
		this->Physics()->SetAngularVelocity(Vector3(0.0f, 0.0f, 0.0f));
		this->Physics()->SetLinearVelocity(worldChange / dt * 0.5f);
	}
	else
	{
		this->m_LocalTransform.SetPositionVector(worldPos - m_LocalClickOffset);
	}
}

void ObjectMeshDragable::OnMouseUp(float dt, const Vector3& worldPos)
{
	if (this->HasPhysics())
	{
		this->Physics()->SetPosition(worldPos - m_LocalClickOffset);
	}
	else
	{
		this->m_LocalTransform.SetPositionVector(worldPos - m_LocalClickOffset);
	}

	this->m_Colour -= m_MouseDownColOffset;
}