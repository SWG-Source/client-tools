#include "FirstUiBuilder.h"
#include "ObjectEditor.h"
#include "EditUtils.h"
#include "EditorMonitor.h"
#include "ObjectFactory.h"
#include "SerializedObjectBuffer.h"

#include "UIImageStyle.h"
#include "UIManager.h"
#include "UIPage.h"
#include "UISaver.h"
#include "UIUtils.h"

#include <limits.h>
#include <list>
#include <set>

#define MAX_BURROW_DEPTH 1000

#define DRAG_OPERATION_IN_PROGRESS_NAME "DRAG"
#define DRAG_OPERATION_NAME             "Drag Selected Widgets"

namespace ObjectEditorNamespace
{
   enum DRAG_STATE 
   { 
      NO_DRAG
     ,TRANS_DRAG
     ,BOX_SELECT 
     ,BOX_SELECT2
   };

	enum { BOX_SELECT_TOLERANCE=4 };

	template<class T>inline const T clamp(const T &minT, const T &t, const T &maxT)
	{
		return (t < minT) ? minT : ((t > maxT) ? maxT : t);
	}

	class Mousestate
	{
	public:

		Mousestate() { reset(); }

		void reset()
		{
			left = right = middle = false;
			ldown = UIPoint::zero;
			rdown = UIPoint::zero;
			mdown = UIPoint::zero;
			lup = UIPoint::zero;
			rup = UIPoint::zero;
			mup = UIPoint::zero;
			cur = UIPoint::zero;
			last = UIPoint::zero;
		}

		bool Left()   const { return left; }
		bool Right()  const { return right; }
		bool Middle() const { return middle; }

		const UIPoint &LDown() const { return ldown; }
		const UIPoint &RDown() const { return rdown; }
		const UIPoint &MDown() const { return mdown; }

		const UIPoint &LUp()   const { return lup;   }
		const UIPoint &RUp()   const { return rup;   }
		const UIPoint &MUp()   const { return mup;   }

		const UIPoint &Mousestate::LDown(int x, int y)
		{
			ldown.Set(x, y);
			left = true;
			return (cur = ldown);
		}

		const UIPoint &Mousestate::RDown(int x, int y)
		{
			rdown.Set(x, y);
			right = true;
			return (cur = rdown);
		}

		const UIPoint &Mousestate::MDown(int x, int y)
		{
			mdown.Set(x, y);
			middle = true;
			return (cur = mdown);
		}

		const UIPoint &Mousestate::LUp(int x, int y)
		{
			lup.Set(x, y);
			left = false;
			return (cur = lup);
		}

		const UIPoint &Mousestate::RUp(int x, int y)
		{
			rup.Set(x, y);
			right = false;
			return (cur = rup);
		}

		const UIPoint &Mousestate::MUp(int x, int y)
		{
			mup.Set(x, y);
			middle = false;
			return (cur = mup);
		}

		const UIPoint &Mousestate::Cur(int x, int y)
		{
			cur.Set(x, y);
			return cur;
		}

		const UIPoint &Cur() const            { return cur; }
		UIPoint        Last()                 { UIPoint tmp = last; last = cur; return tmp; }
		void           Last(const UIPoint &l) { last = l; }

	private:

		bool left, right, middle;
		UIPoint ldown, rdown, mdown;
		UIPoint lup, rup, mup;
		UIPoint cur, last;
	};

	// ==========================================================

	static int _gridSnap(int i, int step)
	{
		if (step>1)
		{
			const int lower = int(i / step) * step;
			const int higher = lower + step;
			const int ldelta = i - lower;
			const int hdelta = higher - i;
			return (hdelta<ldelta) ? higher : lower;
		}
		else
		{
			return i;
		}
	}

}
using namespace ObjectEditorNamespace;

// ==========================================================

class ObjectEditor::ObjectData
{
public:

	typedef std::vector<ObjectData *> LinkList;

	ObjectData()
	{}
	ObjectData(UIBaseObject &object) 
	:	 m_object(&object)
	{}
	ObjectData(const ObjectData &o) 
	:	 m_object(o.m_object)
		,m_links(o.m_links)
		,m_backLinks(o.m_backLinks)
	{}

	int getExtraRefcount() const { return m_backLinks.size(); }

	void eraseOutgoingLink(ObjectData &link) { _eraseLink(link, m_links); }
	void eraseIncomingLink(ObjectData &link) { _eraseLink(link, m_backLinks); }

	void _eraseLink(ObjectData &link, LinkList &list);

	UIBaseObjectHandle m_object;
	LinkList           m_links;
	LinkList           m_backLinks;
};

// ==========================================================

void ObjectEditor::ObjectData::_eraseLink(ObjectData &link, LinkList &list)
{
	LinkList::iterator i = std::find(list.begin(), list.end(), &link);
	if (i!=list.end())
	{
		list.erase(i);
	}
}

// ==========================================================

enum ObjectEditor::PrimitiveEditType 
{
	 PET_UNKNOWN                = -2
	,PET_BEGIN_EDIT_GROUP       = -1

	,PET_insertObjectSubtree    =  0
	,PET_removeObjectSubtree
	,PET_moveObjectSubtree
	,PET_setObjectProperty
	,PET_moveChild
	
	,PET_NUM_TYPES
};

// ==========================================================

class ObjectEditor::PropertyEdit
{
public:
	
	PropertyEdit() {}
	PropertyEdit(const PropertyEdit &o) 
	:	 m_name(o.m_name)
		,m_originalValue(o.m_originalValue)
		,m_newValue(o.m_newValue)
	{}

	UILowerString m_name;
	UIString      m_originalValue;
	UIString      m_newValue;
};

// ==========================================================

class ObjectEditor::EditPrimitive
{
public:

	EditPrimitive()
	:	 m_type(PET_UNKNOWN)
		,m_insertSubtree(0)
	{}

	~EditPrimitive();

	PrimitiveEditType   m_type;
	IndexPath           m_path;
	union
	{
		const char                            *m_groupDescription;
		SerializedObjectBuffer                *m_insertSubtree;
		SerializedObjectBuffer                *m_removeSubtree;
		IndexPath                             *m_moveSubtree;
		PropertyEdit                          *m_setProperty;
		UIBaseObject::ChildMovementDirection   m_moveChild;
	};
};

// ---------------------------------------------------------

ObjectEditor::EditPrimitive::~EditPrimitive()
{
	switch (m_type)
	{
	case PET_insertObjectSubtree: delete m_insertSubtree;   break;
	case PET_removeObjectSubtree: delete m_removeSubtree;   break;
	case PET_moveObjectSubtree:   delete m_moveSubtree;     break;
	case PET_setObjectProperty:   delete m_setProperty;     break;
	default:break;
	}
}

// ==========================================================

ObjectEditor::ObjectEditor(EditorViews &i_views, ObjectFactory &i_factory)
:	m_views(i_views),
	m_objectFactory(i_factory),
	m_editStackTopFront(m_editStack.end()),
	m_editStackTopBack(m_editStack.end()),
	m_currentHighlight(0),
	m_currentSelections(false),
	m_currentHighlights(false),
	m_currentShowPage(0),
	m_mouseState(new Mousestate),
	m_wantsDrag(false),
	m_snapToGrid(false),
	m_gridSteps(10, 10),
	m_dragState(NO_DRAG),
   m_dragStart(0, 0),
	m_dragCurr(0, 0),
	m_dragLast(0, 0),
	m_burrowDepth(0),
	m_burrowPoint(-9999,-9999),
	m_mouseWheelAccumulator(0)
{
	_beginEditing();
}

// ==========================================================

ObjectEditor::~ObjectEditor()
{
	onDragRequest(false);
	cancel();

	delete m_mouseState;
}

// ==========================================================

void ObjectEditor::_beginEditing()
{
	// -----------------------------

	UIObjectList topLevelObjects;

	_getTopLevelObjects(topLevelObjects);
	if (!topLevelObjects.empty())
	{
		for (UIObjectList::iterator oi=topLevelObjects.begin();oi!=topLevelObjects.end();++oi)
		{
			UIBaseObject *o = *oi;
			if (o->IsA(TUIPage))
			{
				UIPage *p = static_cast<UIPage *>(o);
				p->ForceVisible(false);
			}
		}
	}

	// -----------------------------

	UIPage *RootPage = UIManager::gUIManager().GetRootPage();
	if (RootPage)
	{
		_buildObjectDataMap(*RootPage);
	}

	// -----------------------------

	m_currentShowPage = 0;

	// -----------------------------

	_clearAllSelections();

	// -----------------------------

	if (RootPage)
	{
		m_views.onRootSize(RootPage->GetWidth(), RootPage->GetHeight());
	}
}

// ==========================================================

void ObjectEditor::_buildObjectDataMap(UIBaseObject &root)
{
	_r_addObjectData(root);

	ObjectData *objData = _getObjectData(root);
	assert(objData);
	if (objData)
	{
		_r_linkObjectData(*objData);
		_r_checkObjectData(*objData);
	}
	// else ERROR????
}

// ==========================================================

void ObjectEditor::_freeObjectData(UIBaseObject &object)
{
	UI2EditorObjectMap::iterator mi = m_objectData.find(&object);
	if (mi==m_objectData.end())
	{
		return;
	}

	ObjectData *const objData = &mi->second;

	LinkList::iterator li;

	// ------------------------------------------------------------------
	// outgoing links
	for (li=objData->m_links.begin();li!=objData->m_links.end();++li)
	{
		ObjectData *l = *li;
		l->eraseIncomingLink(*objData);
	}
	// ------------------------------------------------------------------

	// ------------------------------------------------------------------
	// incoming links
	for (li=objData->m_backLinks.begin();li!=objData->m_backLinks.end();++li)
	{
		ObjectData *l = *li;
		l->eraseOutgoingLink(*objData);
	}
	// ------------------------------------------------------------------

	m_objectData.erase(mi);
}

// ==========================================================

void ObjectEditor::_freeSubtreeData(UIBaseObject &object)
{
	UIObjectList children;
	EditUtils::getChildren(children, object);
	for (UIObjectList::iterator ci=children.begin();ci!=children.end();++ci)
	{
		UIBaseObject *const c = *ci;
		_freeSubtreeData(*c);
	}

	_freeObjectData(object);
}

// ==========================================================

ObjectEditor::ObjectData *ObjectEditor::_getObjectData(UIBaseObject &object)
{
	UI2EditorObjectMap::iterator mi = m_objectData.find(&object);
	if (mi!=m_objectData.end())
	{
		return &mi->second;
	}
	else
	{
		return 0;
	}
}

// ==========================================================

ObjectEditor::ObjectData *ObjectEditor::_getParentData(ObjectData &objectData)
{
	UIBaseObject *object = objectData.m_object;
	assert(object);
	UIBaseObject *parent = object->GetParent();
	return (parent) ? _getObjectData(*parent) : (ObjectData *)0;
}

// ==========================================================

void ObjectEditor::_getChildData(LinkList &o_children, ObjectData &objectData)
{
	UIBaseObject *object = objectData.m_object;
	assert(object);
	UIObjectList children;
	EditUtils::getChildren(children, *object);
	for (UIObjectList::iterator ci=children.begin();ci!=children.end();++ci)
	{
		UIBaseObject *c = *ci;
		ObjectData *cData = _getObjectData(*c);
		if (cData)
		{
			o_children.push_back(cData);
		}
	}
}

// ==========================================================

void ObjectEditor::_getSubtreeLinks(LinkList &o_outgoingLinks, LinkList &o_incomingLinks, ObjectData &rootData)
{
	std::list<ObjectData *> pending;

	pending.push_back(&rootData);
	while (!pending.empty())
	{
		ObjectData *data = pending.front();
		pending.pop_front();

		UIBaseObject *const object = data->m_object;

		LinkList::iterator li;

		// ------------------------------------------------------------------
		// outgoing links
		for (li=data->m_links.begin();li!=data->m_links.end();++li)
		{
			ObjectData *const link = *li;

			if (!_isDescendant(*link, rootData))
			{
				o_outgoingLinks.push_back(link);
			}
		}
		// ------------------------------------------------------------------

		// ------------------------------------------------------------------
		// incoming links
		for (li=data->m_backLinks.begin();li!=data->m_backLinks.end();++li)
		{
			ObjectData *const link = *li;

			if (!_isDescendant(*link, rootData))
			{
				o_incomingLinks.push_back(link);
			}
		}
		// ------------------------------------------------------------------

		// ------------------------------------------------------------------
		// push children onto queue
		{
			LinkList children;
			_getChildData(children, *data);
			for (li=children.begin();li!=children.end();++li)
			{
				pending.push_back(*li);
			}
		}
		// ------------------------------------------------------------------
	}
}

// ==========================================================

bool ObjectEditor::_isDescendant(ObjectData &object, ObjectData &potentialAncestor)
{
	for (ObjectData *parent=_getParentData(object);parent;parent=_getParentData(*parent))
	{
		if (parent==&potentialAncestor)
		{
			return true;
		}
	}
	return false;
}

// ==========================================================

void ObjectEditor::_r_addObjectData(UIBaseObject &root)
{
	m_objectData[&root] = ObjectData(root);

	// -------------------------------------------

	UIObjectList children;
	EditUtils::getChildren(children, root);
	for (UIObjectList::iterator ci=children.begin();ci!=children.end();++ci)
	{
		UIBaseObject *const o=*ci;
		_r_addObjectData(*o);
	}
}

// ==========================================================

void ObjectEditor::_r_linkObjectData(ObjectData &rootData)
{
	UIBaseObject *object = rootData.m_object;

	// -------------------------------------------------------------------

	UIPropertyNameVector linkProperties;
	object->GetLinkPropertyNames(linkProperties);
	UIString value;
	for (UIPropertyNameVector::iterator lpi=linkProperties.begin();lpi!=linkProperties.end();++lpi)
	{
		UILowerString &name = *lpi;
		if (object->GetProperty(name, value))
		{
			UIBaseObject *l = object->GetObjectFromPath(value);
			if (l)
			{
				ObjectData *linkData = _getObjectData(*l);

				assert(linkData);
				if (linkData)
				{
					rootData.m_links.push_back(linkData);
					linkData->m_backLinks.push_back(&rootData);
				}
				// else ERROR??
			}
		}
	}

	// -------------------------------------------------------------------
	{
		UIObjectList children;
		EditUtils::getChildren(children, *object);
		for (UIObjectList::iterator ci=children.begin();ci!=children.end();++ci)
		{
			UIBaseObject *const o=*ci;
			ObjectData *objectData = _getObjectData(*o);
			assert(objectData);
			if (objectData)
			{
				_r_linkObjectData(*objectData);
			}
			// else ERROR?
		}
	}
}

// ==========================================================

void ObjectEditor::_r_checkObjectData(ObjectData &rootData)
{
#ifndef NDEBUG
	UIBaseObject *object = rootData.m_object;

	int allowedReferences = 1 + rootData.getExtraRefcount();
	if (object->GetParent())
	{
		allowedReferences++;
	}

	// -------------------------------------------------------------------

	{
		UIObjectList children;
		EditUtils::getChildren(children, *object);
		for (UIObjectList::iterator ci=children.begin();ci!=children.end();++ci)
		{
			UIBaseObject *const o=*ci;
			ObjectData *objectData = _getObjectData(*o);
			assert(objectData);
			if (objectData)
			{
				_r_checkObjectData(*objectData);
			}
			// else ERROR?
		}
	}
#else
#endif
}

// ==========================================================

void ObjectEditor::_clearAllSelections()
{
	_lockDialogs();

	UIObjectSet &sels = _sels();
	while (!sels.empty())
	{
		_applyDeselect(*sels.front());
	}

	_unlockDialogs();
}

// ==========================================================

void ObjectEditor::_getTopLevelObjects(UIObjectList &o_list)
{
	o_list.clear();

	UIPage *RootPage = UIManager::gUIManager().GetRootPage();
	if (RootPage)
	{
		RootPage->GetChildren(o_list);
	}
}

// ==========================================================

UIBaseObject *ObjectEditor::_getAnchorSelection(UITypeID typeRestriction)
{
	if (_sels().empty())
	{
		return 0;
	}
	UIBaseObject *anchor = _sels().back();
	if (typeRestriction!=TUIObject
		&& !anchor->IsA(typeRestriction)
		)
	{
		return 0;
	}
	else
	{
		return anchor;
	}
}

// ==========================================================

void ObjectEditor::_getSelections(UIObjectVector &o_objects)
{
	for (UIObjectSet::iterator si=_sels().begin();si!=_sels().end();++si)
	{
		UIBaseObject *o=*si;
		o_objects.push_back(o);
	}
}

// ==========================================================

void ObjectEditor::_getSelectionsNoAnchor(UIObjectVector &o_objects, UITypeID typeRestriction)
{
	if (_sels().size()<2)
	{
		return;
	}

	UIObjectSet::iterator siend = _sels().end();
	--siend;
	for (UIObjectSet::iterator si=_sels().begin();si!=siend;++si)
	{
		UIBaseObject *o=*si;
		if (  typeRestriction==TUIObject
			|| o->IsA(typeRestriction)
			)
		{
			o_objects.push_back(o);
		}
	}
}

// ==========================================================

void ObjectEditor::_getSelectionRoots(UIObjectList &o_roots)
{
	for (UIObjectSet::iterator si=_sels().begin();si!=_sels().end();++si)
	{
		UIBaseObject *o=*si;

		bool skip=false;
		UIBaseObject *p = o->GetParent();
		for (p = o->GetParent();p;p=p->GetParent())
		{
			if (isSelected(*p))
			{
				skip=true;
				break;
			}
		}
		if (!skip)
		{
			o_roots.push_back(o);
		}
	}
}

// ==========================================================

void ObjectEditor::_getSelectionWidgetRoots(UIWidgetVector &o_roots)
{
	for (UIObjectSet::iterator si=_sels().begin();si!=_sels().end();++si)
	{
		UIBaseObject *o=*si;
		if (!o->IsA(TUIWidget))
		{
			continue;
		}

		bool skip=false;
		UIBaseObject *p = o->GetParent();
		for (p = o->GetParent();p;p=p->GetParent())
		{
			if (!p->IsA(TUIWidget))
			{
				continue;
			}
			if (isSelected(*p))
			{
				skip=true;
				break;
			}
		}
		if (!skip)
		{
			o_roots.push_back(UI_ASOBJECT(UIWidget,  o));
		}
	}
}

// ==========================================================

void ObjectEditor::addMonitor(EditorMonitor &i_monitor)
{
	MonitorList::iterator mi = std::find(m_monitors.begin(), m_monitors.end(), &i_monitor);
	if (mi!=m_monitors.end())
	{
		return;
	}
	else
	{
		m_monitors.push_back(&i_monitor);
	}

	// ---------------------------------------------------

	i_monitor.onEditReset();

	// ---------------------------------------------------

	UIBaseObject *root = UIManager::gUIManager().GetRootPage();
	i_monitor.onEditInsertSubtree(*root, EM_INSERT_LAST);

	// ---------------------------------------------------

	for (UIObjectSet::iterator oi=_sels().begin();oi!=_sels().end();++oi)
	{
		i_monitor.onSelect(*(*oi), true);
	}
}

// ==========================================================

void ObjectEditor::removeMonitor(EditorMonitor &i_monitor)
{
	MonitorList::iterator mi = std::find(m_monitors.begin(), m_monitors.end(), &i_monitor);
	if (mi!=m_monitors.end())
	{
		m_monitors.erase(mi);

		i_monitor.onEditReset();
	}
}

// ==========================================================

void ObjectEditor::setGridSteps(const UIPoint &i_newSteps)
{
	m_gridSteps.x = clamp(UIScalar(1), i_newSteps.x, UIScalar(1024));
	m_gridSteps.y = clamp(UIScalar(1), i_newSteps.y, UIScalar(1024));
}

// ==========================================================

void ObjectEditor::gridClosest(UIPoint &o_snapPoint, const UIPoint &point)
{
	o_snapPoint.x = _gridSnap(point.x, m_gridSteps.x);
	o_snapPoint.y = _gridSnap(point.y, m_gridSteps.y);
}

// ==========================================================

void ObjectEditor::onLeftButtonDown(int flags, int x, int y)
{
   m_mouseState->LDown(x, y);

	if (m_dragState==TRANS_DRAG)
	{
      cancelDrag();
	}
   else
	{
      _beginDrag(UIPoint(x, y), BOX_SELECT, flags);
	}
}

// ==========================================================

void ObjectEditor::onLeftButtonUp(int flags, int x, int y)
{
   m_mouseState->LUp(x, y);
   UIPoint mpos;

	if (m_dragState==BOX_SELECT)
	{
      _endDrag();
	}
   else if (m_dragState==BOX_SELECT2)
   {
		_selectHighlighted((flags&MK_CONTROL) ? SEL_ACCUMULATE : SEL_ASSIGN);
      _endDrag();
   }
}

// ==========================================================

void ObjectEditor::onRightButtonDown(int flags, int x, int y)
{
   m_mouseState->RDown(x, y);

	if (m_dragState==NO_DRAG)
   {  
		//otherwise start a drag, if we have not already.
      _beginDrag(m_mouseState->RDown(), TRANS_DRAG, flags); 
   }
}

// ==========================================================

void ObjectEditor::onRightButtonUp(int flags, int x, int y)
{
   m_mouseState->RUp(x, y);

   if (m_dragState==TRANS_DRAG)
	{
      _endDrag();
	}
}

// ==========================================================

void ObjectEditor::onMiddleButtonDown(int flags, int x, int y)
{
   m_mouseState->MDown(x, y);
   m_views.setCapture();
   //m_views.setCursor(CT_HAND);
}

// ==========================================================

void ObjectEditor::onMiddleButtonUp(int flags, int x, int y)
{
   m_mouseState->MUp(x, y);
   m_views.releaseCapture();
   m_views.setCursor(EditorViews::CT_Normal);
}

// ==========================================================

void ObjectEditor::onMouseMove(int flags, int x, int y)
{
   UIPoint last=m_mouseState->Cur();
   UIPoint mpos=m_mouseState->Cur(x, y);

   if (m_dragState!=NO_DRAG)
	{
      _drag(last, mpos-last);
	}
   else
	{
      _move(mpos, last,flags);
	}
}

// ==========================================================

// returns false if mouse wheel event was not used.
bool ObjectEditor::onMouseWheel(int flags, int x, int y, int dz)
{
	m_mouseWheelAccumulator+=dz;

	if (m_dragState==NO_DRAG)
	{
		if (m_mouseWheelAccumulator>=WHEEL_DELTA || m_mouseWheelAccumulator<=-WHEEL_DELTA)
		{
			int steps = m_mouseWheelAccumulator / WHEEL_DELTA;
			m_mouseWheelAccumulator -= steps * WHEEL_DELTA;
			burrow(steps);
		}
		return true;
	}

   return false;
}

// ==========================================================

void ObjectEditor::burrow(int delta)
{
	m_burrowPoint=m_mouseState->Last();
   m_burrowDepth+=delta;
   m_burrowDepth=_findNextPreSelection(m_burrowPoint); 
}

// ==========================================================

bool ObjectEditor::_canDrag()
{
	return m_wantsDrag && hasSelections() && getSelectionCount()==1;
}

// ==========================================================

void ObjectEditor::_beginDrag(const UIPoint &i_pos, int i_drag_state, int key_flags)
{
	UIPoint pos = i_pos;
 
   switch (i_drag_state)
   {
   case TRANS_DRAG: 
   {
   } break;

   case BOX_SELECT:
	{
      _selectHighlighted((key_flags&MK_CONTROL) ? SEL_TOGGLE : SEL_ASSIGN); //always attempt to select when the mouse button goes down
	} break;

   default:
      if (m_dragState!=NO_DRAG)
		{
         _endDrag();
		}
      return;
   }

   assert(m_dragState==NO_DRAG);
   if (m_dragState!=NO_DRAG)                           
	{
      _endDrag();
	}

   m_dragState=i_drag_state;
   m_dragStart=pos;
	m_dragLast.Set(0,0);

   if (m_dragState==TRANS_DRAG)
   {
		m_views.setCursor(EditorViews::CT_SizeAll);
   }

   m_views.setCapture();
}

// ==========================================================

void ObjectEditor::_drag(const UIPoint &i_win_pos, const UIPoint &i_win_vec)
{
   UIPoint pos, vec, mag;

   switch (m_dragState)
   {
   case TRANS_DRAG:
   {
      m_dragCurr=i_win_pos + i_win_vec - m_dragStart;   
   } break;

   case BOX_SELECT:
	{
      m_dragCurr=i_win_pos;

		UIPoint dragVector = m_dragCurr - m_dragStart;
		int dragDistance = dragVector.x*dragVector.x + dragVector.y*dragVector.y;
      if (dragDistance>=BOX_SELECT_TOLERANCE*BOX_SELECT_TOLERANCE)
      {
         m_dragState=BOX_SELECT2;
         break;
      }
      else
         return;
	}

   case BOX_SELECT2:
   {
		m_views.setCursor(EditorViews::CT_SizeNWSE);

      m_dragCurr=i_win_pos;

      UIRect cam_box;

		if (m_dragStart.x <= m_dragCurr.x)
		{
			cam_box.left  = m_dragStart.x;
			cam_box.right = m_dragCurr.x;
		}
		else
		{
			cam_box.right = m_dragStart.x;
			cam_box.left  = m_dragCurr.x;
		}

		if (m_dragStart.y <= m_dragCurr.y)
		{
			cam_box.top    = m_dragStart.y;
			cam_box.bottom = m_dragCurr.y;
		}
		else
		{
			cam_box.bottom = m_dragStart.y;
			cam_box.top    = m_dragCurr.y;
		}

      UIObjectVector pickObjects;
      _pickObjects(pickObjects, cam_box, false);

		_clearHighlights();

		for (UIObjectVector::iterator oi=pickObjects.begin();oi!=pickObjects.end();++oi)
		{
         _highlight(*(*oi), SEL_ACCUMULATE);
		}

      m_dragLast=m_dragCurr;
   } break;

   default:
      return;
   }

	// -------------------------------------------------------

   _applyDrag();

	// -------------------------------------------------------
	// Redraw views.  since dragging can generate a number
	// of updates / painting etc - make the call synchronous
	// to ensure that the dragged objects track the mouse
	// exactly.  
	bool synchronous = m_dragState==TRANS_DRAG;
   _redrawViews(synchronous);
}

// ==========================================================

void ObjectEditor::_applyDrag()
{
   switch (m_dragState)
   {
   case TRANS_DRAG:
	{
		// -------------------------------------------------------

		const char *undo = getCurrentUndoOperation();
		if (undo && strcmp(undo, DRAG_OPERATION_IN_PROGRESS_NAME)==0)
		{
			_endEditOperation(false, false);
		}

		_beginEditOperation(DRAG_OPERATION_IN_PROGRESS_NAME, false);

		// -------------------------------------------------------

		if (m_snapToGrid)
		{
	      gridClosest(m_dragCurr, m_dragCurr);
		}

      if (m_dragCurr!=m_dragLast)
      {
			//UIPoint delta = m_dragCurr-m_dragLast;

			UIWidgetVector widgetRoots;
			_getSelectionWidgetRoots(widgetRoots);

			for (UIWidgetVector::iterator si=widgetRoots.begin();si!=widgetRoots.end();++si)
			{
				UIWidget *w=*si;
				//_edit_translateWidget(*w, delta);
				_eh_translateWidget(*w, m_dragCurr);
			}
         m_dragLast=m_dragCurr;
      }
	} return;

   }
}

// ==========================================================

void ObjectEditor::_endDrag()
{
	if (m_dragState==TRANS_DRAG)
	{
		const char *undo = getCurrentUndoOperation();
		if (undo && strcmp(undo, DRAG_OPERATION_IN_PROGRESS_NAME)==0)
		{
			_renameEditOperation(DRAG_OPERATION_NAME);
			_endEditOperation(true, false);
		}
	}
	else if (m_dragState==BOX_SELECT2)
   {
      _clearHighlights();
		_redrawViews();
   }

   m_dragState = NO_DRAG;

	m_views.setCursor(EditorViews::CT_Normal);
   m_views.releaseCapture();
}

// ==========================================================

int ObjectEditor::_findNextPreSelection(const UIPoint &i_win_pos)
{
	UIObjectVector potentialSelections;
	_pickObjects(potentialSelections, i_win_pos, false);
	if (!potentialSelections.empty())
	{
	   int returnValue=m_burrowDepth;

		returnValue = m_burrowDepth % potentialSelections.size();
		if (returnValue<0)
		{
			returnValue+=potentialSelections.size();
		}

		UIBaseObject *pick = potentialSelections[returnValue];
		_highlight(*pick, SEL_ASSIGN);

		return returnValue;
	}
	else
	{
		_clearHighlights();
		return 0;
	}
}

// ==========================================================

void ObjectEditor::_move(const UIPoint &i_win_pos, const UIPoint &i_prev_pos, int flags)
{
   //we move the mouse, we reset the click depth
   //If we move the mouse enough to change the current preselection..
	if (m_burrowDepth!=0)
	{
		UIPoint delta=i_win_pos-m_burrowPoint;
		if (delta.x<0)
		{
			delta.x=-delta.x;
		}
		if (delta.y<0)
		{
			delta.y=-delta.y;
		}
		if (delta.x>1 || delta.y>1)
		{
			m_burrowDepth=0;
		}
	}

	if (m_dragState==NO_DRAG)
	{
      _findNextPreSelection(i_win_pos);
	}
}

// ==========================================================

void ObjectEditor::_dragView(const UIPoint &i_win_pos, const UIPoint &i_prev_pos,int flags)
{
	_redrawViews(false);
}

// ==========================================================

void ObjectEditor::_pickObjects(UIObjectVector &o_objects, const UIPoint &pt, bool includeTopPage)
{
	o_objects.clear();
	if (m_currentShowPage)
	{
		if (includeTopPage)
		{
			_r_pickObjects(o_objects, *m_currentShowPage, pt);
		}
		else
		{
			UIObjectList children;
			m_currentShowPage->GetChildren(children);
			for (UIObjectList::iterator ci=children.begin();ci!=children.end();++ci)
			{
				UIBaseObject *c = (*ci);
				_r_pickObjects(o_objects, *c, pt);
			}
		}
	}
}

// ==========================================================

void ObjectEditor::_pickObjects(UIObjectVector &o_objects, const UIRect &box, bool includeTopPage)
{
	o_objects.clear();
	if (m_currentShowPage)
	{
		if (includeTopPage)
		{
			_r_pickObjects(o_objects, *m_currentShowPage, box);
		}
		else
		{
			UIObjectList children;
			m_currentShowPage->GetChildren(children);
			for (UIObjectList::iterator ci=children.begin();ci!=children.end();++ci)
			{
				UIBaseObject *c = (*ci);
				_r_pickObjects(o_objects, *c, box);
			}
		}
	}
}

// ==========================================================

void ObjectEditor::_r_pickObjects(UIObjectVector &o_objects, UIBaseObject &root, const UIPoint &pt)
{
	if (root.IsA(TUIWidget))
	{
		UIWidget *w = UI_ASOBJECT(UIWidget,  &root);

		if (!w->IsVisible())
		{
			return;
		}

		UIRect worldRect;
		w->GetWorldRect(worldRect);

		if (worldRect.ContainsPoint(pt))
		{
			o_objects.push_back(&root);
		}
	}

	UIObjectList children;
	EditUtils::getChildren(children, root);
	for (UIObjectList::iterator ci=children.begin();ci!=children.end();++ci)
	{
		UIBaseObject *c = (*ci);
		_r_pickObjects(o_objects, *c, pt);
	}
}

// ==========================================================

void ObjectEditor::_r_pickObjects(UIObjectVector &o_objects, UIBaseObject &root, const UIRect &box)
{
	if (root.IsA(TUIWidget))
	{
		UIWidget *w = UI_ASOBJECT(UIWidget,  &root);

		UIRect worldRect;
		w->GetWorldRect(worldRect);

		const bool selContains = 
				worldRect.left   >= box.left
			&& worldRect.top    >= box.top
			&& worldRect.right  <= box.right
			&& worldRect.bottom <= box.bottom
		;

		if (selContains)
		{
			o_objects.push_back(&root);
		}
	}

	UIObjectList children;
	EditUtils::getChildren(children, root);
	for (UIObjectList::iterator ci=children.begin();ci!=children.end();++ci)
	{
		UIBaseObject *c = (*ci);
		_r_pickObjects(o_objects, *c, box);
	}
}

// ==========================================================

void ObjectEditor::_getSelectionBox(UIRect &o_box)
{
	if (m_dragStart.x <= m_dragCurr.x)
	{
		o_box.left  = m_dragStart.x;
		o_box.right = m_dragCurr.x;
	}
	else
	{
		o_box.right = m_dragStart.x;
		o_box.left  = m_dragCurr.x;
	}

	if (m_dragStart.y <= m_dragCurr.y)
	{
		o_box.top    = m_dragStart.y;
		o_box.bottom = m_dragCurr.y;
	}
	else
	{
		o_box.bottom = m_dragStart.y;
		o_box.top    = m_dragCurr.y;
	}
}

// ==========================================================

void ObjectEditor::cancel()
{
   if (m_dragState!=NO_DRAG)
	{
      cancelDrag();
	}

	/*
   else
   {
      _clearAllSelections();
		_redrawViews();
   }
	*/
}

// ==========================================================

void ObjectEditor::cancelDrag()
{
	if (m_dragState==NO_DRAG)
	{
		return;
	}

   switch (m_dragState)
   {
   case TRANS_DRAG:
	{
		const char *undo = getCurrentUndoOperation();
		if (undo && strcmp(undo, DRAG_OPERATION_IN_PROGRESS_NAME)==0)
		{
			_endEditOperation(false, false);
		}
	} break;

   case BOX_SELECT:
   case BOX_SELECT2:
      _clearHighlights();
      break;
   };

	_redrawViews();
   _endDrag();
}

// ==========================================================

void ObjectEditor::onDeactivate()
{
	cancel();
	onDragRequest(false);
}

// ==========================================================

void ObjectEditor::onDragRequest(bool i_wantsDrag)
{
	if (i_wantsDrag!=m_wantsDrag)
	{
		m_wantsDrag=i_wantsDrag;
		_redrawViews();
	}
}

// ==========================================================

bool ObjectEditor::isSelected(UIBaseObject &o)
{
	return _sels().contains(&o);
}

// ==========================================================

void ObjectEditor::toggleSelect(UIBaseObject &o)
{
	if (isSelected(o))
	{
		_applyDeselect(o);
	}
	else
	{
		_applySelect(o);
	}
}

// ==========================================================

void ObjectEditor::deselect(UIBaseObject &o, bool doChildren)
{
	_applyDeselect(o);

	if (doChildren)
	{
		UIObjectList children;
		EditUtils::getChildren(children, o);

		for (UIObjectList::iterator ci=children.begin();ci!=children.end();++ci)
		{
			deselect(*(*ci), true);
		}
	}
}

// ==========================================================

void ObjectEditor::select(UIBaseObject &o, SelectionOperation op, bool doChildren)
{
	if (op==SEL_ASSIGN)
	{
      _clearAllSelections();
		op=SEL_ACCUMULATE;
	}

	_select(o, op);

	if (doChildren)
	{
		UIObjectList children;
		EditUtils::getChildren(children, o);

		for (UIObjectList::iterator ci=children.begin();ci!=children.end();++ci)
		{
			select(*(*ci), op, true);
		}
	}
}

// ==========================================================

void ObjectEditor::_select(UIBaseObject &o, int selection_op)
{
	switch (selection_op)
	{
	case SEL_ACCUMULATE:
		_applySelect(o);
		break;

	case SEL_ASSIGN:
      _clearAllSelections();
		_applySelect(o);
		break;

	case SEL_TOGGLE:
		toggleSelect(o);
		break;
	}
}

// ==========================================================

void ObjectEditor::_select(const UIObjectVector &i_objectList, int selection_op)
{
	if (selection_op==SEL_ASSIGN)
	{
      _clearAllSelections();
	}

	for (UIObjectVector::const_iterator oi=i_objectList.begin();oi!=i_objectList.end();++oi)
	{
		UIBaseObject &o = *(*oi);

		switch (selection_op)
		{
		case SEL_ACCUMULATE:
		case SEL_ASSIGN:
			_applySelect(o);
			break;
		case SEL_TOGGLE:
			toggleSelect(o);
			break;
		}
	}

}

// ==========================================================

void ObjectEditor::_applySelect(UIBaseObject &o)
{
	if (_sels().push_back(&o))
	{
		_notifySelect(o, true);
		_redrawViews();
	}
}

// ==========================================================

void ObjectEditor::_applyDeselect(UIBaseObject &o)
{
	if (_sels().erase(&o))
	{
		_notifySelect(o, false);
		_redrawViews();
	}
}

// ==========================================================

void ObjectEditor::_selectHighlighted(int selection_op)
{
	UIObjectVector highLights;
	highLights.reserve(m_currentHighlights.size());

	for (UIObjectSet::iterator oi=m_currentHighlights.begin();oi!=m_currentHighlights.end();++oi)
	{
		highLights.push_back(*oi);
	}

	_select(highLights, selection_op);
	_clearHighlights();
}

// ==========================================================

void ObjectEditor::_lockDialogs()
{
	for (MonitorList::iterator mi=m_monitors.begin();mi!=m_monitors.end();++mi)
	{
		(*mi)->lock();
	}
}

// ==========================================================

void ObjectEditor::_unlockDialogs()
{
	for (MonitorList::iterator mi=m_monitors.begin();mi!=m_monitors.end();++mi)
	{
		(*mi)->unlock();
	}
}

// ==========================================================

void ObjectEditor::_notifyInsertSubtree(UIBaseObject &rootObject, UIBaseObject *previousSibling)
{
	for (MonitorList::iterator mi=m_monitors.begin();mi!=m_monitors.end();++mi)
	{
		EditorMonitor *m=*mi;
		m->onEditInsertSubtree(rootObject, previousSibling);
	}
}

// ==========================================================

void ObjectEditor::_notifyRemoveSubtree(UIBaseObject &subTree)
{
	if (&subTree == m_currentHighlight)
	{
		m_currentHighlight=0;
	}

	for (MonitorList::iterator mi=m_monitors.begin();mi!=m_monitors.end();++mi)
	{
		EditorMonitor *m=*mi;
		m->onEditRemoveSubtree(subTree);
	}
}

// ==========================================================

void ObjectEditor::_notifySelect(UIBaseObject &object, bool selected)
{
	for (MonitorList::iterator mi=m_monitors.begin();mi!=m_monitors.end();++mi)
	{
		EditorMonitor *m=*mi;
		m->onSelect(object, selected);
	}
}

// ==========================================================

void ObjectEditor::_notifyPropertyChange(const UIObjectVector &i_objectList, const UILowerString &i_name)
{
	_lockDialogs();
	for (UIObjectVector::const_iterator oi=i_objectList.begin();oi!=i_objectList.end();++oi)
	{
		UIBaseObject *o = *oi;
		_notifyPropertyChange(*o, i_name);
	}
	_unlockDialogs();
}

// ==========================================================

void ObjectEditor::_notifyPropertyChange(UIBaseObject &object, const UILowerString &i_name)
{
	const char *name = i_name.c_str();
	for (MonitorList::iterator mi=m_monitors.begin();mi!=m_monitors.end();++mi)
	{
		EditorMonitor *m=*mi;
		m->onEditSetObjectProperty(object, i_name.c_str());
	}

	UIPage *RootPage = UIManager::gUIManager().GetRootPage();
	if (RootPage)
	{
		if (  &object==RootPage
			&& i_name==UILowerString("Size")
			)
		{
			m_views.onRootSize(RootPage->GetWidth(), RootPage->GetHeight());
		}
	}
}

// ==========================================================

void ObjectEditor::_notifyMoveSubtree(UIBaseObject &subTree, UIBaseObject *previousSibling, UIBaseObject *oldParent)
{
	for (MonitorList::iterator mi=m_monitors.begin();mi!=m_monitors.end();++mi)
	{
		EditorMonitor *m=*mi;
		m->onEditMoveSubtree(subTree, previousSibling, oldParent);
	}
}

// ==========================================================

void ObjectEditor::setCurrentlyVisiblePage(UIBaseObject *o)
{
	if (m_currentShowPage)
	{
		m_currentShowPage->ForceVisible(false);
	}

	if (o)
	{
		UIBaseObject *topLevelObject = EditUtils::getTopLevelObject(o);
		if (topLevelObject && topLevelObject->IsA(TUIPage))
		{
			m_currentShowPage = static_cast<UIPage *>(topLevelObject);
			m_currentShowPage->ForceVisible(true);
		}
	}
	else
	{
		m_currentShowPage=0;
	}

	_redrawViews();
}

// ==========================================================

bool ObjectEditor::isTopLevelPage(UIBaseObject &o)
{
	UIPage *rootPage = UIManager::gUIManager().GetRootPage();
	UIBaseObject *parent = o.GetParent();

	const bool isTop = (parent && parent==rootPage);

	return isTop;
}

// ==========================================================

UIBaseObject *ObjectEditor::getDefaultParent()
{
	UIPage *RootPage = UIManager::gUIManager().GetRootPage();
	UIBaseObject *selectedParent = hasSelections() ? m_currentSelections.back() : RootPage;
	return selectedParent;
}

// ==========================================================

UIBaseObject *ObjectEditor::getAnchorSelection() const
{
	return (hasSelections()) ? getSelections().back() : (UIBaseObject *)0;
}

// ==========================================================

void ObjectEditor::clearSelections()
{
	_clearAllSelections();
}

// ==========================================================

void ObjectEditor::selectDescendants()
{
	if (!hasSelections())
	{
		return;
	}

	_lockDialogs();

	UIObjectVector currentSelections;
	currentSelections.reserve(getSelectionCount());
	for (UIObjectSet::iterator si=m_currentSelections.begin();si!=m_currentSelections.end();++si)
	{
		currentSelections.push_back(*si);
	}

	UIObjectVector subTree;
	for (UIObjectVector::iterator oi=currentSelections.begin();oi!=currentSelections.end();++oi)
	{
		UIBaseObject *o=*oi;

		subTree.clear();
		EditUtils::getSubTree(*o, subTree);
		_select(subTree, SEL_ACCUMULATE);
	}

	_unlockDialogs();
}

// ==========================================================

void ObjectEditor::selectAncestors()
{
	if (!hasSelections())
	{
		return;
	}

	_lockDialogs();

	UIObjectVector currentSelections;
	currentSelections.reserve(getSelectionCount());
	for (UIObjectSet::iterator si=m_currentSelections.begin();si!=m_currentSelections.end();++si)
	{
		currentSelections.push_back(*si);
	}

	UIPage *rootPage = UIManager::gUIManager().GetRootPage();

	UIObjectVector subTree;
	for (UIObjectVector::iterator oi=currentSelections.begin();oi!=currentSelections.end();++oi)
	{
		UIBaseObject *o=*oi;

		if (!o)
		{
			continue;
		}
		for (o = o->GetParent();o && o!=rootPage;o = o->GetParent())
		{
			_select(*o, SEL_ACCUMULATE);
		}
	}

	_unlockDialogs();
}

// ==========================================================

bool ObjectEditor::getSelectionBox(UIRect &o_selectionBox)
{
	if (m_dragState==BOX_SELECT2)
	{
		_getSelectionBox(o_selectionBox);
		return true;
	}
	else
	{
		return false;
	}
}

// ==========================================================

bool ObjectEditor::getSelectionDragBox(UIRect &o_selectionBox)
{
	m_wantsDrag=true;

	if (_canDrag())
	{
		bool first=true;
		UIRect dragBox;
		for (UIObjectSet::iterator oi=_sels().begin();oi!=_sels().end();++oi)
		{
			const UIBaseObject *const o = *oi;
			if (!o->IsA(TUIWidget))
			{
				continue;
			}
			const UIWidget *w = UI_ASOBJECT(UIWidget, o);
			if (first)
			{
				dragBox = w->GetWorldRect();
				first=false;
			}
			else
			{
				dragBox.Extend(w->GetWorldRect());
			}
		}
		if (first)
		{
			return false;
		}
		else
		{
			o_selectionBox = dragBox;
			return true;
		}
	}
	else
	{
		return false;
	}
}

// ==========================================================

bool ObjectEditor::alignSelections(AlignmentDirection alignmentDirection)
{
	if (getSelectionCount()<2)
	{
		return false;
	}

	// -----------------------------------------------
	// make sure a drag operation is not in progress.
	cancelDrag(); 
	// -----------------------------------------------

	switch (alignmentDirection)
	{
	case AD_TOP:     case AD_LEFT:   
	case AD_BOTTOM:  case AD_RIGHT:  
	case AD_WIDTH:   case AD_HEIGHT:
	case AD_CENTERX: case AD_CENTERY:  
		return _edit_alignSelections(alignmentDirection);

	default: 
		return false;
	}
}

// ==========================================================

bool ObjectEditor::deleteSelections()
{
	// -----------------------------------------------
	// make sure a drag operation is not in progress.
	cancelDrag(); 
	// -----------------------------------------------

	return _edit_deleteSelections();
}

// ==========================================================

bool ObjectEditor::renameObject(UIBaseObject *obj, const char *newName)
{
	// -----------------------------------------------
	// make sure a drag operation is not in progress.
	cancelDrag(); 
	// -----------------------------------------------

	const UIBaseObject * const root = UIManager::gUIManager().GetRootPage();
	if (!obj || (obj == root))
	{
		return false;
	}
	return _edit_renameObject(obj, newName);
}

// ==========================================================

UIBaseObject *ObjectEditor::insertNewObject(unsigned menuID)
{
	// -----------------------------------------------------------------------------
	// make sure a drag operation is not in progress.
	cancelDrag(); 
	// -----------------------------------------------------------------------------

	UIPage *RootPage = UIManager::gUIManager().GetRootPage(); assert(RootPage);
	UIBaseObject *selectedParent = hasSelections() ? m_currentSelections.back() : RootPage;

	UIBaseObjectHandle NewObject(m_objectFactory.newObject(menuID, selectedParent));

	if (!NewObject)
		return 0;

	// -----------------------------------------------------------------------------

	_beginEditOperation("Insert New Object");

	// -----------------------------------------------------------------------------

	UIBaseObject *Container  = 0;
	UIBaseObject *parent;

	if (NewObject->IsA(TUIImageFrame))
	{
		// -----------------------------------------------------------------------------
		// Special case image frames so that they automatically create a style
		// when they are created (unless they are already being inserted into a style)

		// -----------------------------------------------------------------------------
		// Try for the existing ImageStyle parent first...
		for (parent=selectedParent;parent;parent=parent->GetParent())
		{
			if (parent->IsA(TUIImageStyle))
			{
				NewObject = _eh_insertObjectSubtree(*NewObject, *parent, EM_INSERT_FIRST); assert(NewObject);
				Container = parent;
				break;
			}			
		}

		if (!Container) // If there is no existing ImageStyle ancestor....
		{
			// ----------------------------------------------------------------
			// None of the selected parent or its ancestors are ImageStyles.
			// We must create our own.
			UIBaseObjectHandle newImageStyle(new UIImageStyle);

			// ----------------------------------------------------------------
			// Attempt to insert our new image style into the selected
			// parent or any of its ancestors...
			for (parent=selectedParent;parent;parent=parent->GetParent())
			{
				const UITypeID parenthoodType = EditUtils::getParenthoodType(*parent);
				if (  parenthoodType==TUINumTypes
					|| !EditUtils::isAdoptionAllowed(*newImageStyle, parenthoodType)
					)
				{
					continue;
				}

				UIBaseObject *const insertion = _eh_insertObjectSubtree(*newImageStyle, *parent, EM_INSERT_FIRST);
				if (insertion)
				{
					newImageStyle=insertion;
					Container = parent;
					break;
				}
			}
			// ----------------------------------------------------------------

			// ----------------------------------------------------------------
			// If the previous insertion failed, try one more time with the root.
			if (!Container)
			{
				UIBaseObject *const insertion = _eh_insertObjectSubtree(*newImageStyle, *RootPage, EM_INSERT_FIRST);
				if (!insertion)
				{
					// TODO ERROR!
				}
				newImageStyle=insertion;
				Container = RootPage;
			}
			// ----------------------------------------------------------------

			// ----------------------------------------------------------------
			// Now that we've found a home for the new ImageStyle, apply default 
			// properties to the new container.
			_eh_applyDefaultPropertiesToObject(*newImageStyle, m_objectFactory.getDefaultPropertiesManager());

			// ----------------------------------------------------------------
			// Insert the new ImageFrame into the new ImageStyle we created.
			NewObject = _eh_insertObjectSubtree(*NewObject, *newImageStyle, EM_INSERT_LAST); assert(NewObject);

			// ----------------------------------------------------------------
			// Apply default properties after we insert the object into the tree
			// so that it can correctly link to other objects 
			_eh_applyDefaultPropertiesToObject(*NewObject, m_objectFactory.getDefaultPropertiesManager());

			// ----------------------------------------------------------------
			// Switch this pointer to the created container so the right object 
			// gets loaded to the tree
			NewObject = newImageStyle;
		}
	}
	else // all other object types.
	{
		// -----------------------------------------------------------
		// Attempt to add to selected parent (or its anscestors)
		for (parent=selectedParent;parent;parent=parent->GetParent())
		{
			const UITypeID parenthoodType = EditUtils::getParenthoodType(*parent);
			if (  parenthoodType==TUINumTypes
				|| !EditUtils::isAdoptionAllowed(*NewObject, parenthoodType)
				)
			{
				continue;
			}

			UIBaseObject *const insertion = _eh_insertObjectSubtree(*NewObject, *parent, EM_INSERT_FIRST);
			if (insertion)
			{
				NewObject = insertion;
				Container = parent;
				break;
			}
		}
		// -----------------------------------------------------------

		// -----------------------------------------------------------
		// If the previous insertion failed, try one more time with the root.
		if (!Container)
		{
			UIBaseObject *const insertion = _eh_insertObjectSubtree(*NewObject, *RootPage, EM_INSERT_FIRST);
			if (!insertion)
			{
				// TODO ERROR!
			}
			NewObject=insertion;
			Container = RootPage;
		}
		// -----------------------------------------------------------

		// -----------------------------------------------------------
		// Apply default properties after we insert the object into the tree
		// so that it can correctly link to other objects 
		_eh_applyDefaultPropertiesToObject(*NewObject, m_objectFactory.getDefaultPropertiesManager());
	}

	// -----------------------------------------------------------

	if (NewObject)
	{
		/*
		parent = NewObject->GetParent();
		if (parent)
		{
			if (parent->CanChildMove(NewObject, UIBaseObject::ChildMovementDirection::Top))
			{
				_ep_moveChild(*NewObject, UIBaseObject::ChildMovementDirection::Top);
			}
		}
		*/

		_endEditOperation(true);

		clearSelections();
		select(*NewObject);
	}
	else
	{
		_endEditOperation(false);
	}

	return NewObject;
}

// ==========================================================

bool ObjectEditor::setObjectProperty(const UIObjectVector &i_objects, const UILowerString &i_name, const UIString &i_value)
{
	// -----------------------------------------------
	// make sure a drag operation is not in progress.
	cancelDrag(); 
	// -----------------------------------------------

	return _edit_setObjectProperty(i_objects, i_name, i_value);
}

// ==========================================================

bool ObjectEditor::setObjectsParent(const UIObjectVector &i_objects, UIBaseObject &parent)
{
	// -----------------------------------------------
	// make sure a drag operation is not in progress.
	cancelDrag(); 
	// -----------------------------------------------

	UITypeID parenthoodType = EditUtils::getParenthoodType(parent);
	if (parenthoodType==TUINumTypes)
	{
		return false;
	}

	UIPage *RootPage = UIManager::gUIManager().GetRootPage();
	if (!RootPage)
	{
		return false;
	}

	UIObjectVector::const_iterator oi;
	for (oi=i_objects.begin();oi!=i_objects.end();++oi)
	{
		UIBaseObject *o=*oi;

		if (o==RootPage)
		{
			return false;
		}

		if (!EditUtils::isAdoptionAllowed(*o, parenthoodType))
		{
			return false;
		}
	}

	return _edit_moveObjectSubtrees(i_objects, parent);
}

// ==========================================================

void ObjectEditor::copySelectionsToClipboard(bool cut)
{
	_edit_copySelectionsToClipboard(cut);
}

// ==========================================================

void ObjectEditor::pasteFromClipboard()
{
	_edit_pasteSelectionsFromClipboard();
}

// ==========================================================

const char *ObjectEditor::getCurrentUndoOperation()
{
	if (m_editStackTopFront!=m_editStackTopBack)
	{
		assert(m_editStackTopFront->m_type==PET_BEGIN_EDIT_GROUP);
		return m_editStackTopFront->m_groupDescription;
	}
	else
	{
		return 0;
	}
}

// ==========================================================

const char *ObjectEditor::getCurrentRedoOperation()
{
	if (m_editStackTopBack!=m_editStack.end())
	{
		assert(m_editStackTopBack->m_type==PET_BEGIN_EDIT_GROUP);
		return m_editStackTopBack->m_groupDescription;
	}
	else
	{
		return 0;
	}
}

// ==========================================================

void ObjectEditor::undo()
{
	while (m_editStackTopFront!=m_editStackTopBack)
	{
		m_editStackTopBack--;
		_undo(*m_editStackTopBack);
	}
	if (m_editStackTopFront!=m_editStack.begin())
	{
		do
		{
			--m_editStackTopFront;
		} while (m_editStackTopFront->m_type!=PET_BEGIN_EDIT_GROUP);
	}
   _redrawViews(false);
}

// ==========================================================

void ObjectEditor::redo()
{
	m_editStackTopFront=m_editStackTopBack;
	while (m_editStackTopBack!=m_editStack.end())
	{
		_do(*m_editStackTopBack);
		++m_editStackTopBack;
		if (m_editStackTopBack->m_type==PET_BEGIN_EDIT_GROUP)
		{
			break;
		}
	}
   _redrawViews(false);
}

// ==========================================================

void ObjectEditor::_clearHighlights()
{
	m_currentHighlights.clear();
}

// ==========================================================

void ObjectEditor::_highlight(UIBaseObject &obj, int selectionOp)
{
	switch (selectionOp)
	{
	case SEL_ACCUMULATE:
		m_currentHighlights.insert(&obj);
		break;
	case SEL_ASSIGN:
		_clearHighlights();
		m_currentHighlights.insert(&obj);
		break;
	case SEL_TOGGLE:
		if (m_currentHighlights.contains(&obj))
		{
			m_currentHighlights.erase(&obj);
		}
		else
		{
			m_currentHighlights.push_back(&obj);
		}
		break;
	}
}

// ==========================================================
// Edits - basic edit operations.
// ==========================================================

bool ObjectEditor::_edit_renameObject(UIBaseObject *obj, const char *newName)
{
	_beginEditOperation("Rename object");

	UIString newNameVal(Unicode::narrowToWide(newName));
	const bool success = _ep_setObjectProperty(*obj, UIBaseObject::PropertyName::Name, newNameVal);
	_endEditOperation(success);
	return success;
}

// ==========================================================

bool ObjectEditor::_edit_setObjectProperty(const UIObjectVector &i_objectList, const UILowerString &i_name, const UIString &i_value)
{
	_beginEditOperation("Multiple Set Property");

	for (unsigned i=0;i<i_objectList.size();++i)
	{
		_ep_setObjectProperty(*i_objectList[i], i_name, i_value);
	}

	_endEditOperation(true);
	return true;
}

// ==========================================================

bool ObjectEditor::_edit_moveObjectSubtrees(const UIObjectVector &i_objects, UIBaseObject &parent)
{
	_beginEditOperation("Multiple Set Parent");

	for (UIObjectVector::const_iterator oi=i_objects.begin();oi!=i_objects.end();++oi)
	{
		UIBaseObject *const o = *oi;

		if (!_ep_moveObjectSubtree(*o, parent))
		{
			_endEditOperation(false);
			return false;
		}
	}

	_endEditOperation(true);
	return true;
}

// ==========================================================

bool ObjectEditor::_edit_alignSelections(AlignmentDirection alignmentDirection)
{
	UIBaseObject *anchor = _getAnchorSelection(TUIWidget);
	if (!anchor)
	{
		return false;
	}

	UIObjectVector toMove;
	_getSelectionsNoAnchor(toMove, TUIWidget);
	if (toMove.empty())
	{
		return false;
	}

	_beginEditOperation("Align Selections");
	bool success=true;

	const UIWidget *const anchorWidget   = UI_ASOBJECT(UIWidget,  anchor);
	const UIRect anchorRect = anchorWidget->GetWorldRect();

	for (UIObjectVector::iterator oi=toMove.begin();oi!=toMove.end();++oi)
	{
		UIWidget *const w = UI_ASOBJECT(UIWidget, *oi);
		const UIRect wrect = w->GetWorldRect();

		UIRect lrect = w->GetRect();
		switch (alignmentDirection)
		{
		case AD_TOP:
		{
			const int deltaY = anchorRect.top - wrect.top;
			lrect.top+=deltaY;
			lrect.bottom+=deltaY;
		} break;
		case AD_LEFT:
		{
			const int deltaX = anchorRect.left - wrect.left;
			lrect.left+=deltaX;
			lrect.right+=deltaX;
		} break;
		case AD_BOTTOM:
		{
			const int deltaY = anchorRect.bottom - wrect.bottom;
			lrect.top+=deltaY;
			lrect.bottom+=deltaY;
		} break;
		case AD_RIGHT:
		{
			const int deltaX = anchorRect.right - wrect.right;
			lrect.left+=deltaX;
			lrect.right+=deltaX;
		} break;
		case AD_WIDTH:
		{
			lrect.right = lrect.left + anchorRect.Width();
		} break;
		case AD_HEIGHT:
		{
			lrect.bottom = lrect.top + anchorRect.Height();
		} break;
		case AD_CENTERX:
		{
			const int anchorCenterX = anchorRect.GetCenter().x;
			const int widgetCenterX = wrect.GetCenter().x;
			const int deltaX = anchorCenterX - widgetCenterX;
			lrect.left+=deltaX;
			lrect.right+=deltaX;
		} break;
		case AD_CENTERY:
		{
			const int anchorCenterY = anchorRect.GetCenter().y;
			const int widgetCenterY = wrect.GetCenter().y;
			const int deltaY = anchorCenterY - widgetCenterY;
			lrect.top+=deltaY;
			lrect.bottom+=deltaY;
		} break;
		}

		if (!_eh_setObjectRect(*w, lrect))
		{
			success=false;
			break;
		}
	}

	_endEditOperation(success);
	return success;
}

// ==========================================================

bool ObjectEditor::_edit_deleteSelections()
{
	_beginEditOperation("Delete Selections");

	UIObjectList toDelete;
	_getSelectionRoots(toDelete);

	bool success = _eh_deleteObjectSubtrees(toDelete);
	_endEditOperation(success);
	return success;
}

// ==========================================================

void ObjectEditor::_edit_copySelectionsToClipboard(bool cut)
{
	UIObjectList selectionRoots;
	_getSelectionRoots(selectionRoots);

	if (selectionRoots.empty())
	{
		return;
	}

	std::list<SerializedObjectBuffer> buffers;

	for (UIObjectList::iterator oi=selectionRoots.begin();oi!=selectionRoots.end();++oi)
	{
		UIBaseObject *o = *oi;
		if (o->GetParent())
		{
			buffers.push_back();
			buffers.back().serialize(*o);
		}
	}

	if (!buffers.empty())
	{
		m_views.copyObjectBuffersToClipboard(buffers);

		if (cut)
		{
			_beginEditOperation("Delete selections (Cut)");

			// delete objects
			const bool success = _eh_deleteObjectSubtrees(selectionRoots);

			_endEditOperation(success);
		}
	}
}

// ==========================================================

void ObjectEditor::_edit_pasteSelectionsFromClipboard()
{
	_beginEditOperation("Paste From Clipboard");
	bool success=true;

	UIObjectList selectionRoots;
	_getSelectionRoots(selectionRoots);
	for (UIObjectList::iterator oi=selectionRoots.begin();oi!=selectionRoots.end();++oi)
	{
		UIBaseObject *o = *oi;
		if (!_eh_pasteSelectionsFromClipboard(*o))
		{
			success=false;
			break;
		}
	}

	_endEditOperation(success);
}

// ==========================================================
// Edit Helpers
// ==========================================================

UIBaseObject *ObjectEditor::_eh_insertObjectSubtree(UIBaseObject &subTree, UIBaseObject &parent, UIBaseObject *previousSibling)
{
	SerializedObjectBuffer *buffer = new SerializedObjectBuffer;
	buffer->serialize(subTree);
	return _ep_insertObjectSubtree(buffer, parent, previousSibling);
}

// ==========================================================

UIBaseObject *ObjectEditor::_eh_insertObjectSubtree(const SerializedObjectBuffer &subTree, UIBaseObject &parent, UIBaseObject *previousSibling)
{
	return _ep_insertObjectSubtree(new SerializedObjectBuffer(subTree), parent, previousSibling);
}

// ==========================================================

bool ObjectEditor::_eh_deleteObjectSubtrees(UIObjectList &toDelete)
{
	_lockDialogs();

	bool success=true;
	for (UIObjectList::iterator oi=toDelete.begin();oi!=toDelete.end();++oi)
	{
		if (!_ep_removeObjectSubtree(*(*oi)))
		{
			success=false;
			break;
		}
	}

	_unlockDialogs();

	return success;
}

// ==========================================================

bool ObjectEditor::_eh_setObjectRect(UIBaseObject &object, const UIRect &NewRect)
{
	UISize NewSize (NewRect.Size ());
	UIString sizeText, locText;
	if (!UIUtils::FormatPoint(sizeText, NewSize))
	{
		return false;
	}
	if (!UIUtils::FormatPoint(locText, UIPoint(NewRect.left, NewRect.top)))
	{
		return false;
	}

	const bool success=
		   _ep_setObjectProperty(object, UIWidget::PropertyName::Size,     sizeText)
		&& _ep_setObjectProperty(object, UIWidget::PropertyName::Location, locText )
	;

	return success;
}

// ==========================================================

bool ObjectEditor::_eh_setObjectSize(UIBaseObject &object, const UIPoint &newSize)
{
	UIString sizeText;
	if (!UIUtils::FormatPoint(sizeText, newSize))
	{
		return false;
	}

	const bool success = _ep_setObjectProperty(object, UIWidget::PropertyName::Size, sizeText);

	return success;
}

// ==========================================================

bool ObjectEditor::_eh_translateWidget(UIWidget &w, const UIPoint &delta)
{
	UIPoint localPos = w.GetLocation();
	localPos+=delta;
	UIString locText;
	if (!UIUtils::FormatPoint(locText, localPos))
	{
		return false;
	}

	const bool success= _ep_setObjectProperty(w, UIWidget::PropertyName::Location, locText);

	return success;
}

// ==========================================================

bool ObjectEditor::_eh_pasteSelectionsFromClipboard(UIBaseObject &NewParentObject)
{
	// -----------------------------------------------------------------
	// check to see if parent can take children.
	UITypeID parenthoodType = EditUtils::getParenthoodType(NewParentObject);
	if (parenthoodType==TUINumTypes)
	{
		// TODO ERROR
		return false;
	}
	// -----------------------------------------------------------------

	// -----------------------------------------------------------------
	std::list<SerializedObjectBuffer> buffers;
	if (!m_views.pasteObjectBuffersFromClipboard(buffers))
	{
		return false;
	}
	// -----------------------------------------------------------------

	// -----------------------------------------------------------------
	UIObjectVector newObjects;
	for (std::list<SerializedObjectBuffer>::iterator bi=buffers.begin();bi!=buffers.end();++bi)
	{
		SerializedObjectBuffer &b=*bi;

		UIBaseObject *newObject = _eh_insertObjectSubtree(b, NewParentObject, EM_INSERT_LAST);
		if (newObject)
		{
			newObjects.push_back(newObject);
		}
		else
		{
			// TODO ERROR
			//char buffer[1028];
			//sprintf(buffer, "%d object%s could not be pasted because they could not be contained in the selected object.", FailedCopies, FailedCopies == 1 ? "" : "s" );
			//MessageBox( 0, buffer, 0, MB_OK );
			return false;
		}
	}
	// -----------------------------------------------------------------

	// -----------------------------------------------------------------
	// Link again once all objects are inserted.  This catches cases where
	// pasted objects link to each other.  For example, if I paste two linked
	// sibling objects, the link would not necessarily succeed until after both 
	// had been inserted.
	for (UIObjectVector::iterator oi=newObjects.begin();oi!=newObjects.end();++oi)
	{
		UIBaseObject *const o = *oi;
		o->Link();
	}
	// -----------------------------------------------------------------

	return true;
}

// ==========================================================

bool ObjectEditor::_eh_applyDefaultPropertiesToObject(UIBaseObject &object, const DefaultObjectPropertiesManager &i_defaultsManager)
{
	typedef DefaultObjectPropertiesManager::DefaultObjectPropertiesList DefaultObjectPropertiesList;
	typedef DefaultObjectPropertiesManager::StringMap                   StringMap;

	// -------------------------------------------------

	const DefaultObjectPropertiesList &defaults = i_defaultsManager.getDefaultProperties();

	const char *typeName = object.GetTypeName();

	for (DefaultObjectPropertiesList::const_iterator i = defaults.begin(); i != defaults.end(); ++i)
	{
		if (_stricmp(typeName, i->TypeName.c_str()) == 0)
		{
			for (StringMap::const_iterator j = i->Properties.begin(); j != i->Properties.end(); ++j)
			{
				_ep_setObjectProperty(object, UILowerString(j->first), j->second);
			}
			break;
		}
	}

	// -------------------------------------------------

	if (object.IsA(TUIWidget))
	{
		UIWidget * const wid = static_cast<UIWidget *>(&object);	
		const UIWidget * const parent = wid->GetParentWidget ();

		if (parent)
		{
			UISize size = wid->GetSize ();
			const UISize & parentSize = parent->GetSize ();
			bool sizeChanged=false;
			if (parentSize.x<size.x)
			{
				size.x=parentSize.x;
				sizeChanged=true;
			}
			if (parentSize.y<size.y)
			{
				size.y=parentSize.y;
				sizeChanged=true;
			}
			if (sizeChanged)
			{
				_eh_setObjectSize(*wid, size);
			}
		}
	}

	// -------------------------------------------------
	
	return true;
}

// ==========================================================
// Edit Primitives / Undo-Redo
// ==========================================================

bool ObjectEditor::_do(EditPrimitive &i_edit)
{
	switch (i_edit.m_type)
	{
	case PET_insertObjectSubtree:  return _do_insertObjectSubtree(i_edit, true)!=0;
	case PET_removeObjectSubtree:  return _do_removeObjectSubtree(i_edit);
	case PET_moveObjectSubtree:    return _do_moveObjectSubtree(i_edit);
	case PET_setObjectProperty:    return _do_setObjectProperty(i_edit);
	case PET_moveChild:            return _do_moveChild(i_edit);
	default: break;
	}

	return false;
}

// ==========================================================

bool ObjectEditor::_undo(EditPrimitive &i_edit)
{
	switch (i_edit.m_type)
	{
	case PET_insertObjectSubtree:  return _undo_insertObjectSubtree(i_edit);
	case PET_removeObjectSubtree:  return _undo_removeObjectSubtree(i_edit)!=0;
	case PET_moveObjectSubtree:    return _undo_moveObjectSubtree(i_edit);
	case PET_setObjectProperty:    return _undo_setObjectProperty(i_edit);
	case PET_moveChild:            return _undo_moveChild(i_edit);
	default: break;
	}

	return false;
}

// ==========================================================

void ObjectEditor::_beginEditOperation(const char *i_description, bool lockDialogs)
{
	while (m_editStackTopBack!=m_editStack.end())
	{
		m_editStackTopBack=m_editStack.erase(m_editStackTopBack);
	}

	// -----------------------------------------------

	m_editStackTopFront = m_editStack.insert(m_editStackTopBack);
	EditPrimitive &group = *m_editStackTopFront;
	group.m_type = PET_BEGIN_EDIT_GROUP;
	group.m_groupDescription = i_description;

	if (lockDialogs)
	{
		_lockDialogs();
	}
}

// ==========================================================

void ObjectEditor::_renameEditOperation(const char *i_newName)
{
	if (m_editStackTopFront!=m_editStack.end() && m_editStackTopFront->m_type==PET_BEGIN_EDIT_GROUP)
	{
		m_editStackTopFront->m_groupDescription = i_newName;
	}
}

// ==========================================================

void ObjectEditor::_endEditOperation(bool commit, bool lockDialogs)
{
	if (!commit)
	{
		assert(m_editStackTopBack==m_editStack.end());

		// ----------------------------------------------------
		// undo any primitives that may have been inserted before
		// the operation failed.
		EditPrimitiveList::iterator pi=m_editStackTopBack;
		while (pi!=m_editStackTopFront)
		{
			--pi;
			_undo(*pi);
		}
		// ----------------------------------------------------

		// ----------------------------------------------------
		// erase current operation and find the previous one.
		m_editStack.erase(m_editStackTopFront, m_editStackTopBack);

		m_editStackTopFront=m_editStackTopBack;
		while (m_editStackTopFront!=m_editStack.begin())
		{
			--m_editStackTopFront;
			if (m_editStackTopFront->m_type==PET_BEGIN_EDIT_GROUP)
			{
				break;
			}
		}
		// ----------------------------------------------------
	}

	if (lockDialogs)
	{
		_unlockDialogs();
	}
}

// ==========================================================

UIBaseObject *ObjectEditor::_ep_insertObjectSubtree(SerializedObjectBuffer *subTree, UIBaseObject &parent, UIBaseObject *previousSibling)
{
	// ---------------------------------------------
	// create edit primitive
	m_editStack.push_back();
	EditPrimitive &ep = m_editStack.back();
	ep.m_type = PET_insertObjectSubtree;
	EditUtils::getIndexPath(ep.m_path, parent);

	int childIndex;
	if (previousSibling==EM_INSERT_LAST)
	{
		childIndex=EditUtils::getChildCount(parent);
	}
	else if (previousSibling==EM_INSERT_FIRST)
	{
		childIndex=0;
	}
	else
	{
		childIndex = EditUtils::getChildIndex(*previousSibling) + 1;
	}

	ep.m_path.push_back(childIndex);
	ep.m_insertSubtree = subTree;
	// ---------------------------------------------

	UIBaseObject *newObject = _do_insertObjectSubtree(ep, true);
	if (!newObject)
	{
		m_editStack.pop_back();
	}
	else
	{
		assert(newObject->GetParent());
		assert(EditUtils::getChildIndex(*newObject)==childIndex);
	}

	return newObject;
}

// ==========================================================

bool ObjectEditor::_ep_removeObjectSubtree(UIBaseObject &subTree)
{
	if (&subTree==UIManager::gUIManager().GetRootPage())
	{
		return false;
	}
	assert(!EditUtils::isTransientWidget(subTree));

	// --------------------------------------------------

	ObjectData *rootData = _getObjectData(subTree);
	if (!rootData)
	{
		return false;
	}
	LinkList incomingLinks, outgoingLinks;
	_getSubtreeLinks(outgoingLinks, incomingLinks, *rootData);

	// --------------------------------------------------
	if (!incomingLinks.empty())
	{
		char buf [512];
		_snprintf(
			buf, 
			sizeof (buf) - 1, 
			"%s has %d extra outstanding references.", 
			subTree.GetName().c_str(), 
			incomingLinks.size()
			);
		MessageBox(0, buf, "UiBuilder", MB_OK) ;
		return false;
	}
	// --------------------------------------------------

	// --------------------------------------------------
	if (&subTree==getCurrentlyVisiblePage())
	{
		// TODO - select something else to show?
		setCurrentlyVisiblePage(0);
	}
	// --------------------------------------------------

	UIBaseObject *theParent = subTree.GetParent();
	UIBaseObjectHandle hold(&subTree);

	// ---------------------------------------------
	// create edit primitive
	m_editStack.push_back();
	EditPrimitive &ep = m_editStack.back();

	ep.m_type = PET_removeObjectSubtree;
	EditUtils::getIndexPath(ep.m_path, subTree);
	ep.m_removeSubtree = new SerializedObjectBuffer;
	ep.m_removeSubtree->serialize(subTree);
	// ---------------------------------------------

	const bool success=_do_removeObjectSubtree(ep);
	if (!success)
	{
		m_editStack.pop_back();
	}

	//---------------------------------------------------
	return success;
}

// ==========================================================

bool ObjectEditor::_ep_moveObjectSubtree(UIBaseObject &subTree, UIBaseObject &parent)
{
	assert(!EditUtils::isTransientWidget(subTree));

	UIBaseObject *theParent = subTree.GetParent();
	if (!theParent)
	{
		return false;
	}

	UIBaseObjectHandle hold(&subTree);

	// ---------------------------------------------
	// create edit primitive
	m_editStack.push_back();
	EditPrimitive &ep = m_editStack.back();
	ep.m_type = PET_moveObjectSubtree;

	// source (remove) path
	EditUtils::getIndexPath(ep.m_path, subTree);

	// dest (insert) path
	ep.m_moveSubtree = new IndexPath;
	EditUtils::getIndexPath(*ep.m_moveSubtree, parent, &subTree);
	const int childIndex = EditUtils::getChildCount(parent, &subTree);
	ep.m_moveSubtree->push_back(childIndex);
	// ---------------------------------------------

	const bool success=_do_moveObjectSubtree(ep);
	if (!success)
	{
		m_editStack.pop_back();
	}

	// ----------------------------------------

	return true;
}

// ==========================================================

bool ObjectEditor::_ep_setObjectProperty(UIBaseObject &object, const UILowerString &i_name, const UIString &i_value)
{
	// ---------------------------------------------
	// create edit primitive
	m_editStack.push_back();
	EditPrimitive &ep = m_editStack.back();
	ep.m_type = PET_setObjectProperty;
	EditUtils::getIndexPath(ep.m_path, object);
	ep.m_setProperty = new PropertyEdit;
	ep.m_setProperty->m_name = i_name;
	object.GetProperty(i_name, ep.m_setProperty->m_originalValue);
	ep.m_setProperty->m_newValue = i_value;
	// ---------------------------------------------

	const bool success = _do_setObjectProperty(ep);
	if (!success)
	{
		m_editStack.pop_back();
	}
	return success;
}

// ==========================================================

bool ObjectEditor::_ep_moveChild(UIBaseObject &child, UIBaseObject::ChildMovementDirection direction)
{
	// ---------------------------------------------
	// create edit primitive
	m_editStack.push_back();
	EditPrimitive &ep = m_editStack.back();
	ep.m_type = PET_moveChild;
	EditUtils::getIndexPath(ep.m_path, child);
	ep.m_moveChild = direction;
	// ---------------------------------------------

	const bool success = _do_moveChild(ep);
	if (!success)
	{
		m_editStack.pop_back();
	}
	return success;
}

// ==========================================================

UIBaseObject *ObjectEditor::_do_insertObjectSubtree(EditPrimitive &i_edit, bool checkAdoption)
{
	IndexPath::const_iterator iChildIndex = i_edit.m_path.end()-1;
	UIBaseObject *parent = EditUtils::getObjectFromPath(i_edit.m_path.begin(), iChildIndex);
	if (parent)
	{
		// ---------------------------------------------
		UITypeID parenthoodType;
		if (checkAdoption)
		{
			parenthoodType = EditUtils::getParenthoodType(*parent);
			if (parenthoodType==TUINumTypes)
			{
				return 0;
			}
		}
		else
		{
			parenthoodType=TUINumTypes;
		}
		// ---------------------------------------------

		UIBaseObjectHandle o(i_edit.m_insertSubtree->unserialize());
		if (o)
		{
			UIBaseObject *previousSibling;
			if (  (  !checkAdoption
					|| EditUtils::isAdoptionAllowed(*o, parenthoodType)
					)
				&& EditUtils::insertChild(*o, *parent, *iChildIndex, previousSibling)
				)
			{
				assert(EditUtils::getChildIndex(*o)==*iChildIndex);

				o->Link();

				if (o->IsA(TUIWidget))
				{
					static_cast<UIWidget *>(&(*o))->ResetPackSizeInfo();
					static_cast<UIWidget *>(&(*o))->ResetPackLocationInfo();
				}

				_buildObjectDataMap(*o);
				_notifyInsertSubtree(*o, previousSibling);

				return o;
			}
		}
	}
	return 0;
}

// ==========================================================

bool ObjectEditor::_undo_insertObjectSubtree(EditPrimitive &i_edit)
{
	return _do_removeObjectSubtree(i_edit);
}

// ==========================================================

bool ObjectEditor::_do_removeObjectSubtree(EditPrimitive &i_edit)
{
	UIBaseObjectHandle object(EditUtils::getObjectFromPath(i_edit.m_path.begin(), i_edit.m_path.end()));
	if (!object)
	{
		return false;
	}

	UIBaseObject *parent = object->GetParent();
	if (!parent)
	{
		return false;
	}

	//---------------------------------------------------

	{
		UIObjectList subTreeList;
		UIObjectList::iterator oi;

		EditUtils::getSubTree(*object, subTreeList);

		// --------------------------------------------------

		_lockDialogs();

		for (oi=subTreeList.begin();oi!=subTreeList.end();++oi)
		{
			deselect(*(*oi));
		}

		_notifyRemoveSubtree(*object);

		_unlockDialogs();
	}

	//---------------------------------------------------

	bool success =  parent->RemoveChild(object);
	assert(success);

	//---------------------------------------------------

	/*
	allowedReferenceCount--;

	//-- if anything has an outstanding reference to this object, relink the whole tree to be safe
	if (theObjectToDelete->GetRefCount () > allowedReferenceCount)
	{
		//-- set the parent so we can unlink properly
		theObjectToDelete->SetParent (theParent);
		
		//-- don't link the root page's children from here
		RootPage->UIWidget::Link();
		
		if (theObjectToDelete->GetRefCount () > 1)
		{
			UIBaseObject::UIObjectList olist;
			RootPage->GetChildren (olist);
			
			for (UIBaseObject::UIObjectList::const_iterator it = olist.begin (); it != olist.end () && theObjectToDelete->GetRefCount () > 1; ++it)
			{
				UIBaseObject * const child = *it;
				child->Link ();
			}
		}
		theObjectToDelete->SetParent (0);
	}
	*/
	//---------------------------------------------------

	_freeSubtreeData(*object);

	return true;
}

// ==========================================================

UIBaseObject *ObjectEditor::_undo_removeObjectSubtree(EditPrimitive &i_edit)
{
	return _do_insertObjectSubtree(i_edit, false); 
}

// ==========================================================

bool ObjectEditor::_do_moveObjectSubtree(EditPrimitive &i_edit)
{
	return _do_moveObjectSubtree(i_edit.m_path, *i_edit.m_moveSubtree);
}

// ==========================================================

bool ObjectEditor::_undo_moveObjectSubtree(EditPrimitive &i_edit)
{
	return _do_moveObjectSubtree(*i_edit.m_moveSubtree, i_edit.m_path);
}

// ==========================================================

bool ObjectEditor::_do_moveObjectSubtree(const IndexPath &sourcePath, const IndexPath &destPath)
{
	IndexPath::const_iterator iDestChildIndex = destPath.end()-1;
	// ---------------------------------------------

	// ---------------------------------------------
	UIBaseObject *object = EditUtils::getObjectFromPath(sourcePath.begin(), sourcePath.end());
	if (!object)
	{
		return false;
	}
	// ---------------------------------------------

	// ---------------------------------------------
	UIBaseObject *oldParent = object->GetParent();
	if (!oldParent)
	{
		return false;
	}
	// ---------------------------------------------

	// ---------------------------------------------
	UIBaseObject *newParent = EditUtils::getObjectFromPath(destPath.begin(), iDestChildIndex);
	if (!newParent)
	{
		return false;
	}
	// ---------------------------------------------

	// ---------------------------------------------
	UITypeID parenthoodType = EditUtils::getParenthoodType(*newParent);
	if (  parenthoodType==TUINumTypes
		|| !EditUtils::isAdoptionAllowed(*object, parenthoodType)
		)
	{
		return false;
	}
	// ---------------------------------------------

	UIBaseObjectHandle h(object);

	if (!oldParent->RemoveChild(object))
	{
		return false;
	}

	UIBaseObject *previousSibling;
	if (!EditUtils::insertChild(*object, *newParent, *iDestChildIndex, previousSibling))
	{
		EditUtils::insertChild(*object, *oldParent, sourcePath.back(), previousSibling);
		return false;
	}

	assert(EditUtils::getChildIndex(*object)==*iDestChildIndex);

	_notifyMoveSubtree(*object, previousSibling, oldParent);

	return true;
}

// ==========================================================

bool ObjectEditor::_do_setObjectProperty(EditPrimitive &i_edit)
{
	return _do_setObjectProperty(i_edit, i_edit.m_setProperty->m_newValue);
}

// ==========================================================

bool ObjectEditor::_do_setObjectProperty(EditPrimitive &i_edit, const UIString &newValue)
{
	UIBaseObject *object = EditUtils::getObjectFromPath(i_edit.m_path.begin(), i_edit.m_path.end());
	if (!object)
	{
		return false;
	}
	if (!object->SetProperty(i_edit.m_setProperty->m_name, newValue))
	{
		return false;
	}
	_notifyPropertyChange(*object, i_edit.m_setProperty->m_name);
	return true;
}

// ==========================================================

bool ObjectEditor::_undo_setObjectProperty(EditPrimitive &i_edit)
{
	return _do_setObjectProperty(i_edit, i_edit.m_setProperty->m_originalValue);
}

// ==========================================================

bool ObjectEditor::_do_moveChild(EditPrimitive &i_edit)
{
	UIBaseObject *object = EditUtils::getObjectFromPath(i_edit.m_path.begin(), i_edit.m_path.end());
	if (!object)
	{
		return false;
	}
	UIBaseObject *parent = object->GetParent();
	if (!parent)
	{
		return false;
	}
	return parent->MoveChild(object, i_edit.m_moveChild);
}

// ==========================================================

bool ObjectEditor::_undo_moveChild(EditPrimitive &i_edit)
{
	// TODO - I'm not even sure this can reliably be un-done without
	// changes to the ui library.
	return true;
}

// ==========================================================
