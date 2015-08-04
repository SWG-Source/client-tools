#ifndef __OBJECTEDITOR_H__
#define __OBJECTEDITOR_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ObjectEditor.h : header file

#include "UIBaseObject.h"
#include "UIObjectSet.h"
#include "EditorViews.h"
#include "DefaultObjectPropertiesManager.h"

#include <list>
#include <map>

class ObjectFactory;
class UIPage;
class UIWidget;
class EditorMonitor;

namespace ObjectEditorNamespace
{
	class Mousestate;
}

class ObjectEditor
{
public:

	// ---------------------------------------
	typedef UISmartPointer<UIBaseObject>      UIBaseObjectHandle;

	typedef UIBaseObject::UIObjectList        UIObjectList;
	typedef UIBaseObject::UIObjectVector      UIObjectVector;
	typedef ui_stdvector<UIWidget *>::fwd     UIWidgetVector;
	typedef UIBaseObject::UIBaseObjectPointer UIBaseObjectPointer;
	typedef UIBaseObject::UISmartObjectList   UISmartObjectList;

	typedef std::vector<int>                  IndexPath;

	// ---------------------------------------
	// Alignment directoins
	enum AlignmentDirection {
		 AD_TOP
		,AD_LEFT
		,AD_BOTTOM
		,AD_RIGHT
		,AD_WIDTH
		,AD_HEIGHT
		,AD_CENTERX
		,AD_CENTERY
	};

	// ---------------------------------------
	// Selection operations
   enum SelectionOperation {
      SEL_ACCUMULATE
     ,SEL_ASSIGN
     ,SEL_TOGGLE
   };

	// ---------------------------------------

	ObjectEditor(EditorViews &i_views, ObjectFactory &i_factory);
	~ObjectEditor();

	// ---------------------------------------
	// Event monitors
	void addMonitor(EditorMonitor &i_monitor);
	void removeMonitor(EditorMonitor &i_monitor);
	// ---------------------------------------

	// ---------------------------------------
   // Grid
	bool           getSnapToGrid()                           const { return m_snapToGrid; }
	const UIPoint &getGridSteps()                            const { return m_gridSteps; }

	bool           setSnapToGrid(bool x)                           { return (m_snapToGrid = x); }
	void           setGridSteps(const UIPoint &i_newSteps);
   void           gridClosest(UIPoint &o_snapPoint, const UIPoint &point);
	// ---------------------------------------

	// ---------------------------------------
   // User input
   void onLeftButtonDown(int flags, int x, int y);
   void onLeftButtonUp(int flags, int x, int y);
   void onRightButtonDown(int flags, int x, int y);
   void onRightButtonUp(int flags, int x, int y);
   void onMiddleButtonDown(int flags, int x, int y);
   void onMiddleButtonUp(int flags, int x, int y);
   void onMouseMove(int flags, int x, int y);

   // returns false if mouse wheel event was not used.
   bool onMouseWheel(int flags, int x, int y, int dz); 

   void cancel(); // generic context sensitive cancel command (i.e. esc key)
   void cancelDrag();

	void burrow(int delta=1);

	void onDeactivate();
	void onDragRequest(bool i_wantsDrag);
	// ---------------------------------------

	// ---------------------------------------
	UIPage *getCurrentlyVisiblePage() { return m_currentShowPage; }
	void setCurrentlyVisiblePage(UIBaseObject *);
	bool isTopLevelPage(UIBaseObject &);

	UIBaseObject *getDefaultParent();
	// ---------------------------------------

	// ---------------------------------------
	// Selection list management
	bool               hasSelections()      const { return !m_currentSelections.empty(); }
	int                getSelectionCount()  const { return m_currentSelections.size(); }
	const UIObjectSet &getSelections()      const { return m_currentSelections; }
	const UIObjectSet &getHighlights()      const { return m_currentHighlights; }
	UIBaseObject      *getAnchorSelection() const;

	void clearSelections();
	void selectDescendants();
	void selectAncestors();

	bool isSelected(UIBaseObject &o);
	void toggleSelect(UIBaseObject &o);
	void deselect(UIBaseObject &o, bool doChildren=false);
	void select(UIBaseObject &o, SelectionOperation op=SEL_ACCUMULATE, bool doChildren=false);

	bool getSelectionBox(UIRect &o_selectionBox);

	bool getSelectionDragBox(UIRect &o_selectionBox);
	// ---------------------------------------

	// ---------------------------------------
	// User edits.
	bool          alignSelections(AlignmentDirection alignmentDirection);
	bool          deleteSelections();

	bool          renameObject(UIBaseObject *obj, const char *newName);

	UIBaseObject *insertNewObject(unsigned menuID);

	bool          setObjectProperty(const UIObjectVector &i_objects, const UILowerString &i_name, const UIString &i_value);

	bool          setObjectsParent(const UIObjectVector &i_objects, UIBaseObject &parent);

	void          pasteFromClipboard();
	void          copySelectionsToClipboard(bool cut);
	// ---------------------------------------

	// ---------------------------------------
	// Un-do / Re-do
	const char *getCurrentUndoOperation(); // returns NULL is no operation is available.
	const char *getCurrentRedoOperation(); // returns NULL is no operation is available.

	void undo(); 
	void redo();
	// ---------------------------------------

protected:

	class ObjectData;

	typedef std::vector<char> Buffer;
	typedef std::vector<EditorMonitor *> MonitorList;

	typedef UIBaseObject::UIPropertyNameVector UIPropertyNameVector;
	typedef std::vector<ObjectData *> LinkList;
	typedef std::map<UIBaseObject *, ObjectData> UI2EditorObjectMap;

	void _beginEditing();

	// ----------------------------------------------------
	// Support for extended object data.
	void         _buildObjectDataMap(UIBaseObject &root);
	void         _freeObjectData(UIBaseObject &object);
	void         _freeSubtreeData(UIBaseObject &object);
	ObjectData * _getObjectData(UIBaseObject &object);
	ObjectData * _getParentData(ObjectData &objectData);
	void         _getChildData(LinkList &o_children, ObjectData &objectData);
	void         _getSubtreeLinks(LinkList &o_outgoingLinks, LinkList &o_incomingLinks, ObjectData &rootData);
	bool         _isDescendant(ObjectData &object, ObjectData &potentialAncestor);

	void         _r_addObjectData(UIBaseObject &root);
	void         _r_linkObjectData(ObjectData &root);
	void         _r_checkObjectData(ObjectData &root);
	// ----------------------------------------------------

	// ----------------------------------------------------
	void          _getTopLevelObjects(UIObjectList &o_list);
	UIBaseObject *_getAnchorSelection(UITypeID typeRestriction=TUIObject);
	void _getSelections(UIObjectVector &o_objects);
	void _getSelectionsNoAnchor(UIObjectVector &o_objects, UITypeID typeRestriction=TUIObject);
	void _getSelectionRoots(UIObjectList &o_roots);
	void _getSelectionWidgetRoots(UIWidgetVector &o_roots);

	// ---------------------------------------
	// Picking / dragging
	bool _canDrag();

	void _beginDrag(const UIPoint &i_pos, int i_drag_state, int key_flags);
	void _drag(const UIPoint &i_win_pos, const UIPoint &i_win_vec);
	void _applyDrag();
	void _endDrag();

	void _move(const UIPoint &i_win_pos, const UIPoint &i_prev_pos,int flags);
	void _dragView(const UIPoint &i_win_pos, const UIPoint &i_prev_pos,int flags);

	void _pickObjects(UIObjectVector &o_objects, const UIPoint &pt, bool includeTopPage);
	void _pickObjects(UIObjectVector &o_objects, const UIRect &box, bool includeTopPage);

	void _r_pickObjects(UIObjectVector &o_objects, UIBaseObject &root, const UIPoint &pt);
	void _r_pickObjects(UIObjectVector &o_objects, UIBaseObject &root, const UIRect &box);

	void _getSelectionBox(UIRect &o_box);
	// ---------------------------------------

	// ---------------------------------------
	void _clearHighlights();
	int  _findNextPreSelection(const UIPoint &i_win_pos);
	void _highlight(UIBaseObject &obj, int selectionOp);

	// ==========================================================
	// Edits - basic edit operations.
	// An edit operation is a group of one or more edit promitives
	// and represents a single undo/redo operation for the user.
	bool _edit_renameObject(UIBaseObject *obj, const char *newName);
	bool _edit_setObjectProperty(const UIObjectVector &i_objectList, const UILowerString &i_name, const UIString &i_value);

	bool _edit_alignSelections(AlignmentDirection alignmentDirection);

	bool _edit_moveObjectSubtrees(const UIObjectVector &i_objects, UIBaseObject &parent);

	bool _edit_deleteSelections();

	void _edit_copySelectionsToClipboard(bool cut);

	void _edit_pasteSelectionsFromClipboard();
	// ===================================================================

	// ===================================================================
	// Edit helpers
	UIBaseObject *_eh_insertObjectSubtree(UIBaseObject &subTree, UIBaseObject &parent, UIBaseObject *previousSibling);
	UIBaseObject *_eh_insertObjectSubtree(const SerializedObjectBuffer &subTree, UIBaseObject &parent, UIBaseObject *previousSibling);

	bool _eh_deleteObjectSubtrees(UIObjectList &objs);

	bool _eh_setObjectRect(UIBaseObject &object, const UIRect &NewRect);
	bool _eh_setObjectSize(UIBaseObject &object, const UIPoint &newSize);
	bool _eh_translateWidget(UIWidget &w, const UIPoint &delta);

	bool _eh_pasteSelectionsFromClipboard(UIBaseObject &NewParentObject);

	bool _eh_applyDefaultPropertiesToObject(UIBaseObject &object, const DefaultObjectPropertiesManager &i_defaults);

	// ===================================================================

	// ===================================================================
	// Edit primitives and undo/redo logic.
	enum PrimitiveEditType;
	class PropertyEdit;
	class EditPrimitive;
	typedef std::list<EditPrimitive> EditPrimitiveList;

	bool _do(EditPrimitive &i_edit);
	bool _undo(EditPrimitive &i_edit);

	void _beginEditOperation(const char *i_description, bool lockDialogs=true);
	void _renameEditOperation(const char *i_newName);
	void _endEditOperation(bool commit, bool lockDialogs=true);

	UIBaseObject *_ep_insertObjectSubtree(SerializedObjectBuffer *subTree, UIBaseObject &parent, UIBaseObject *previousSibling);
	bool          _ep_removeObjectSubtree(UIBaseObject &subTree);
	bool          _ep_moveObjectSubtree(UIBaseObject &subTree, UIBaseObject &parent);
	bool          _ep_setObjectProperty(UIBaseObject &object, const UILowerString &i_name, const UIString &i_value);
	bool          _ep_moveChild(UIBaseObject &child, UIBaseObject::ChildMovementDirection);

	UIBaseObject *_do_insertObjectSubtree(EditPrimitive &i_edit, bool checkAdoption);
	bool          _undo_insertObjectSubtree(EditPrimitive &i_edit);

	bool          _do_removeObjectSubtree(EditPrimitive &i_edit);
	UIBaseObject *_undo_removeObjectSubtree(EditPrimitive &i_edit);

	bool          _do_moveObjectSubtree(EditPrimitive &i_edit);
	bool          _undo_moveObjectSubtree(EditPrimitive &i_edit);
	bool          _do_moveObjectSubtree(const IndexPath &sourcePath, const IndexPath &destPath);

	bool          _do_setObjectProperty(EditPrimitive &i_edit);
	bool          _do_setObjectProperty(EditPrimitive &i_edit, const UIString &newValue);
	bool          _undo_setObjectProperty(EditPrimitive &i_edit);

	bool          _do_moveChild(EditPrimitive &i_edit);
	bool          _undo_moveChild(EditPrimitive &i_edit);

	// ===================================================================

	// ---------------------------------------
	void _clearAllSelections();

   void _selectHighlighted(int selection_op);

	void _select(UIBaseObject &o, int selection_op);
	void _select(const UIObjectVector &i_objectList, int selection_op);

	void _applySelect(UIBaseObject &o);
	void _applyDeselect(UIBaseObject &o);
	// ---------------------------------------

	// ---------------------------------------
	// dia;ogs / notifications
	void _lockDialogs();
	void _unlockDialogs();
	void _notifyInsertSubtree(UIBaseObject &rootObject, UIBaseObject *previousSibling);
	void _notifyRemoveSubtree(UIBaseObject &object);
	void _notifySelect(UIBaseObject &object, bool selected);
	void _notifyPropertyChange(const UIObjectVector &i_objectList, const UILowerString &i_name);
	void _notifyPropertyChange(UIBaseObject &object, const UILowerString &i_name);
	void _notifyMoveSubtree(UIBaseObject &subTree, UIBaseObject *previousSibling, UIBaseObject *oldParent);
	// ---------------------------------------

	// ---------------------------------------

	void _redrawViews(bool sync=false) { m_views.redrawViews(sync); }

	// ---------------------------------------

	UIObjectSet &_sels() { return m_currentSelections; }

	// ---------------------------------------

	EditorViews                    &m_views;
	ObjectFactory                  &m_objectFactory;
	MonitorList                     m_monitors;

	UI2EditorObjectMap              m_objectData;

	EditPrimitiveList               m_editStack;
	EditPrimitiveList::iterator     m_editStackTopFront, m_editStackTopBack;

	UIBaseObject                   *m_currentHighlight;
	UIObjectSet                     m_currentSelections;
	UIObjectSet                     m_currentHighlights;

	UIPage                         *m_currentShowPage;
	DefaultObjectPropertiesManager  m_defaultPropertiesManager;

	ObjectEditorNamespace::Mousestate *m_mouseState;
	bool                               m_wantsDrag;
   bool                               m_snapToGrid;
	UIPoint                            m_gridSteps;
   int                                m_dragState;
   UIPoint                            m_dragStart;
	UIPoint                            m_dragCurr;
	UIPoint                            m_dragLast;
	int                                m_burrowDepth;
	UIPoint                            m_burrowPoint;
	int                                m_mouseWheelAccumulator;

	// ---------------------------------------
};

#endif
//
