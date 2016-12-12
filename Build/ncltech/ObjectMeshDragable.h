/******************************************************************************
Class: ObjectMeshDragable
Implements: ObjectMesh
Author: Pieran Marris      <p.marris@newcastle.ac.uk> and YOU!
Description:

Example variant of ObjectMesh that makes the object in question clickable by the 
user. This will allow the object to be clicked and dragged around the scene via use of
the ScreenPicker component. The default behaviour is as follows:
	1. Mouse Enter [hover]  -> Set to highlight colour
	2. Mouse Click			-> Set to clicked colour
	3. Mouse Move			-> Move position (either worldtransform or physics->pos depending if the object has physics enabled)
	4. Mouse Up				-> Set back to highlight colour
	5. Mouse Leave			-> Set back to default colour

Probably not the most sophisticated use of mouse interactivity, though if it helps
debug some physics code at some point then it will have served it's purpose.

TODO: Start using shorter/abbreviated class names

		(\_/)
		( '_')
	 /""""""""""""\=========     -----D
	/"""""""""""""""""""""""\
....\_@____@____@____@____@_/

*//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ObjectMesh.h"

class ObjectMeshDragable : public ObjectMesh
{
public:
	ObjectMeshDragable(const std::string& name);
	virtual ~ObjectMeshDragable();

	//Change in colour when mouse is hovering over the object
	void SetMouseOverColourOffset(const Vector4& col_offset);	

	//Change in colour when mouse down
	void SetMouseDownColourOffset(const Vector4& col_offset);	

protected:
	virtual void OnMouseEnter(float dt) override;
	virtual void OnMouseLeave(float dt) override;
	virtual void OnMouseDown(float dt, const Vector3& worldPos) override;
	virtual void OnMouseMove(float dt, const Vector3& worldPos, const Vector3& worldChange) override;
	virtual void OnMouseUp(float dt, const Vector3& worldPos) override;

protected:
	Vector4 m_MouseOverColOffset;
	Vector4 m_MouseDownColOffset;

	Vector3 m_LocalClickOffset;
};