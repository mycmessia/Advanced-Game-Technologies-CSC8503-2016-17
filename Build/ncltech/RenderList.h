/******************************************************************************
Class: RenderList
Implements:
Author: Pieran Marris <p.marris@newcastle.ac.uk>
Description:

Render utility to automatically keep track of (and sort) all opaque and transparent objects
within a given frustum. This is similar to the vector<SceneNode*> used in the Scene Management
tutorial from Graphics for Games, however instead of being created/deleted each frame it will keep a
persistant list of objects. The Scene searches through all objects and only inserts them if they
are not already in the list, and deletes them only after they have left the given view frustum.

This results in a slightly more effecient method of handling render lists. It also means that the
list can be kept permenantly sorted, and only small changes per frame will be required resulting in
a fast insertion sort method that plays heavily on this frame coherency.

The drawback (other than not being implemented very well) is that in order to keep track of
whether an object is already in the list or not a bitwise flag is set on the Object itself. This
results in a hard-limit of 32 renderlists. As the constructor could exceed this value, a factory is
used to create the renderlist, returning false if all renderlists have already been allocated previously.


		(\_/)
		( '_')
	 /""""""""""""\=========     -----D
	/"""""""""""""""""""""""\
....\_@____@____@____@____@_/

*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Object.h"
#include <nclgl\Frustum.h>
#include <nclgl\common.h>
#include <nclgl\Vector3.h>
#include <vector>



//Maximum number of elements added to the renderlist per frame 
// - Will be added on future frames instead to share workload and prevent lock-ups
#define MAX_LIST_CHANGE_PER_FRAME 300

//Sort opaque objects front to back to reduce over drawing - tie up between slow sorting or slow rendering, in the current
//usage the sorting is almost always the bottlekneck and overdraw on the Gfx card is negligible. 
// - Transparent objects however always need to be sorted in order to correctly blend with background objects.
#define SORT_OPAQUE_LIST FALSE 





struct RenderList_Object
{
	float cam_dist_sq;
	Object* target_obj;
};

class RenderList
{
public:
	virtual ~RenderList();

	//RenderList Factory
	//  - Attempts to create new renderlist - returns false if max-renderlists (32) have already previously been allocated.
	static bool AllocateNewRenderList(RenderList** renderlist, bool supportsTransparency); 


	//Updates all current objects 'distance' to camera
	void UpdateCameraWorldPos(const Vector3& cameraPos); 

	//Sort lists based on camera position using insertion sort. With frame coherency,
	// the list should be 'almost' sorted each frame and only a few elements need to be swapped in this function.
	void SortLists(); 

	//Removes all objects no longer inside the frustum
	void RemoveExcessObjects(const Frustum& frustum); 

	//Called when object moves inside the frustum (Inserts via insertion sort)
	void InsertObject(Object* obj); 

	//Misc. Removes a single object from the list, in general just call 'RemoveExcessObjects' and let it remove the object automatically
	void RemoveObject(Object* obj); 

	//Clears the entire list
	void RemoveAllObjects(); 


	//Iterate over each object in the list calling the provided function per-object.
	void RenderOpaqueObjects(const std::function<void(Object*)>& per_object_func);
	void RenderTransparentObjects(const std::function<void(Object*)>& per_object_func);

	//Get the bitmask set per Object in the list
	uint BitMask() { return m_BitMask; }

protected:
	//Keeps a list of the number of unique render lists as to enforce and maintain the hard limit on 32 renderlists.
	static uint g_NumRenderLists; 
	
	
	uint m_NumElementsChanged;

	//Bit mask for renderlist, uint leads to 32 booleans (1's or 0's) thus there is a hardlimit of 31 shadow maps along with the main camera render list. 
	uint m_BitMask;				 

	//If false - all transparent objects will be ignored (maybe shadow render passes?)
	bool m_SupportsTransparancy; 

	//Last provided camera position for sorting/inserting new objects
	Vector3 m_CameraPos;

	//Sorted renderlists of visible objects
	std::vector<RenderList_Object> m_vRenderListOpaque;
	std::vector<RenderList_Object> m_vRenderListTransparent;

private:
	//Private Constructor - Allocate through 'AllocateNewRenderList' factory method
	RenderList();

	//No copying! - the deconstructor updates a global counter of active renderlists so deleting a copy would cause all future renderlist instances to overlap each other
	RenderList(const RenderList& rl) {}
};
