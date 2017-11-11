// ======================================================================
//
// GodClientData.h
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_GodClientData_H
#define INCLUDED_GodClientData_H

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"
#include "sharedMath/Vector.h"
#include "sharedMath/Vector2d.h"
#include "sharedMath/Transform.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkIdArchive.h"
#include "sharedNetworkMessages/AIDebuggingMessages.h"

#include "clientGame/ClientObject.h"

#include "Singleton/Singleton.h"

#include <map>
#include <set>
#include <list>

// ======================================================================

class Camera;
class ClientObject;
class Object;
class GroundScene;
class BoxExtent;
class GodClientIoWin;
class ParticleEffectAppearance;
class VectorArgb;

// ======================================================================
/**
* GodClientData is the singleton which maintains the current set of selected objects,
* the current clipboard of copied object, and provides many utility methods for transforming
* and manipulating objects in the world.
*/
class GodClientData : public MessageDispatch::Emitter, public MessageDispatch::Receiver, public Singleton<GodClientData>
{
public:
	enum RotationPivotType
	{
		RotatePivot_self,
		RotatePivot_selectionCenter,
		RotatePivot_lastSelection
	};

	enum RotationType
	{
		Rotate_none,
		Rotate_yaw,
		Rotate_pitch,
		Rotate_roll
	};

	struct Messages
	{
		static const char* const SELECTION_CHANGED;
		static const char* const SELECTED_OBJECTS_CHANGED;
		static const char* const CLIPBOARD_CHANGED;
		static const char* const GHOSTS_CREATED;
		static const char* const GHOSTS_KILLED;
		static const char* const PALETTES_CHANGED;
	};

	struct SelectedObject
	{
		explicit SelectedObject(ClientObject* theObj) : obj(theObj), ghost(0) {}
		~SelectedObject();
		Watcher<ClientObject> obj;
		Object*               ghost;
	private:
		//disabled
		SelectedObject();
		SelectedObject& operator=(const SelectedObject& rhs);
	};

	struct ClipboardObject
	{
		std::string serverObjectTemplateName;
		std::string sharedObjectTemplateName;
		std::string const & getObjectTemplateName() const;
		Transform   transform;
		NetworkId   networkId;

		ClipboardObject();
		explicit ClipboardObject(const Object& obj);

		Object* findInClientWorld() const;
	};

	struct SphereObject
	{
		Vector    location;
		real      radius;
	};

	struct SphereTreeObject
	{
		NetworkId    networkId;
		int          sphereId;
		SphereObject sphere;
	};

	struct TriggerVolumeSphere
	{
		SphereObject sphere;
		NetworkId    networkId;
	};

	struct AINode
	{
		Vector              m_location;
		int                 m_parent;
		stdvector<int>::fwd m_children;
		stdvector<int>::fwd m_siblings;
		int                 m_type;
		int                 m_level;
	};

	struct AIPath
	{
		stdvector<AIPathInfo_NodeInfo>::fwd m_nodes;
	};

	typedef std::list<SphereTreeObject>              SphereTreeObjectList_t;
	typedef std::list<TriggerVolumeSphere>           TriggerSphereList_t;
	typedef std::list<ClipboardObject*>              ClipboardList_t;
	typedef std::map<int, std::list<ClientObject*> > SelectionGroupMap_t;
	typedef std::list<std::string>                   Palette_t;
	typedef std::map<std::string, Palette_t>         PaletteMap_t;
	typedef std::set<unsigned int>                   RequestIdSet_t;
	typedef std::list<ClientObject*>                 ObjectList_t;
	typedef std::list<SelectedObject*>               SelectedObjectList_t;
	typedef std::list<const ClientObject*>           ObjectWithAxes_t;
	typedef std::map<int, AINode>                    AINodeList_t;
	typedef std::map<NetworkId, AIPath>              AIPathList_t;

	GodClientData();
	~GodClientData();


	void initialize();

	//--- visuals related functions -----------------------------------
	void             draw                       ();
	void             draw                       (const Camera& camera);
	void             receiveMessage             (const MessageDispatch::Emitter& source, const MessageDispatch::MessageBase& message);
	void             cursorScreenPositionChanged(int x, int y);
	Vector2d         getCursorScreenPosition    () const;
	bool             findIntersection_p (const int screenX, const int screenY, const CellProperty*& cellProperty, Vector& intersection_p);
	void             getSphereTreeSnapshot      ();
	void             showTriggerVolumes         ();
	void             clearSpheres               ();
	void             snapToGridDlg              ();
	void             setTransformDlg            ();
	void             toggleShowObjectAxes       (const ClientObject* const obj);
	void             togglePauseParticleSystem  (ClientObject* const obj);
	bool             isParticleSystemSelected   ();

	bool             isBuildingSelected(const Object** buildingSelected=0) const;

	void             setMouseCursorIntersection (const Vector& point);
	Vector           getMouseCursorIntersection () const;

	//--- path display functions --------------------------------------
	void             setRenderAIPaths           (bool i_render);
	void             ignoreTargetPath           (const NetworkId &object);

	//--- palette-related functions -----------------------------------
	void               addPalette          (const std::string& name);
	void               deletePalette       (const std::string& name);
	void               addTemplateToPalette(const std::string& templateName);
	void               setSelectedPalette  (const std::string& name);
	size_t             getNumPalettes      () const;
	Palette_t          getPalette          (int index, std::string& name);
	const std::string& getSelectedPalette  () const;

	//--- selection-related functions ---------------------------------
	void             setSelection                  (ClientObject* obj);
	void             addSelection                  (ClientObject* obj);
	bool             removeSelection               (const ClientObject* obj);
	void             toggleSelection               (ClientObject* obj);
	void             clearSelection                ();
	void             getSelection                  (ObjectList_t& objectList);
	void             replaceSelection              (const ObjectList_t& objectList);	
	bool             getSelectionEmpty             () const;
	void             storeSelection                (int key);
	void             restoreSelection              (int key);
	int              getSelectionSize              ();
	bool             calculateSelectionCenter      (Vector& center, bool ghosts) const;
	bool             calculateClipboardCenter      (const ClipboardList_t& clip, Vector& center) const;
	bool             calculateClipboardBottom(const ClipboardList_t& clip, float& bottom) const;
	bool             getSelectionExtent            (bool ghosts, BoxExtent& extent) const;
	bool             getIsSelectionOrGhost         (const Object& obj, bool& isSelection, bool& isGhost) const;
	void             saveCurrentSelectionAsBrush   (const std::string& brushName);
	void             setSelectionX                 (real x);
	void             setSelectionY                 (real y);
	void             setSelectionZ                 (real z);
	void             setSelectionYaw               (real yaw);
	void             setSelectionPitch             (real pitch);
	void             setSelectionRoll              (real roll);
	void             translateSelection            (real x, real y, bool alongGround);
	void             translateSelectionY           (real y);
	void             scaleSelection                (real dx, real dy);
	void             scaleSelectionY               (real dy);
	void             translateGhosts               (const Vector& v, bool alongGround);
	void             scaleGhosts                   (const Vector& v);
	void             rotateGhosts                  (const real v, RotationType type, RotationPivotType pivotType);
	void             dropGhostsToTerrain           ();
	void             resetGhostsRotations          ();
	void             alignGhostsToTerrain          ();
	void             synchronizeSelectionWithGhosts();
	void             unlockSelectedObjects         ();
	int              killGhosts                    ();
	int              countGhosts                   ();
	bool             moveGhostsFollowingObject     (const NetworkId& id, const Vector& pt, bool alongGround);
	void             setObjectCreationPending      (bool val);
	bool             getObjectCreationPending      ();

	//---clipboard-related functions-----------------------------------
	void             copySelection                 ();
	void             clearClipboard                ();
	bool             getClipboardEmpty             () const;
	void             getClipboard                  (ClipboardList_t& objectList) const;
	void             setCurrentBrush               (const ClipboardList_t& brush);
	void             getCurrentBrush               (ClipboardList_t& brush) const;
	bool             pasteLocationKnown            () const;
	void             setPasteLocation              (const Vector& vec);
	Vector           absorbPasteLocation           ();

	void             toggleDropToTerrain           ();
	bool             isToggleDropToTerrainOn       () const;
	void             toggleAlignToTerrain          ();
	bool             isToggleAlignToTerrainOn      () const;

	const Vector &   getCameraPivotPoint           ();

private:
	real             getObjectDropCollisionHeight  (const Object& obj) const;
	void             dropObjectToTerrain           (Object& obj, real height) const;
	void             alignObjectToTerrain          (Object& obj) const;

	ClientObject*    findSelectionByNetworkId      (const NetworkId& id);
	SelectedObject*  findSelectedObjectByNetworkId (const NetworkId& id);
	void             createGhosts                  ();
	Object*          createGhost                   (ClientObject&obj);
	Vector           snapToGrid                    (const Vector& originalLoc) const;

	void             handleSceneChange             ();

	void             enableShowObjectAxes          (const ClientObject* const obj);
	void             disableShowObjectAxes         (const ClientObject* const obj);
	void             pauseParticleSystem           (ParticleEffectAppearance* particleAppearance) const;
	void             unpauseParticleSystem         (ParticleEffectAppearance* particleAppearance) const;

	void             cleanSelectedObjectList       ();

	void             drawCrossbars                 (const Vector& point, float barSize, const VectorArgb& color);

private:

	///the currently selected objects
	SelectedObjectList_t          m_selectedObjects;
	///the items currently stored in the clipboard
	ClipboardList_t               m_clipboard;
	PaletteMap_t                  m_palettes;
	///stores a number of selections that can be restored(i.e. Ctrl-F1 to store, Shift-F1 to restore)
	SelectionGroupMap_t           m_persistedSelections;
	///a map of the current snapshot of the sphere tree(from the server)
	SphereTreeObjectList_t        m_sphereTreeObjects;
	///list of currently selected object triggers
	TriggerSphereList_t           m_triggerObjectSpheres;
	///a list of currently outstanding server messge requests that we care about
	RequestIdSet_t                m_outstandingRequests;
	///a list of objects that we show the axes on
	ObjectWithAxes_t              m_showAxesObjects;

	///the list of current AI Nodes to represent visually
	AINodeList_t                  m_AINodes;

	///the list of current AI Paths to represent visually
	AIPathList_t                  m_AIPaths;

	///enable or disable AI path rendering
	bool                          m_renderAIPaths;

	///the current "center" point of the camera - what we pivot the camera around
	Vector                        m_pivot;

	Vector                        m_lastPivotCameraPos;

	mutable int                   m_createdCount;

	///pointer to the scene, for conveinance
	GroundScene*                  m_gs;

	///pointer to our custom IOWin
	GodClientIoWin*               m_ioWin;

	///the screen position of the cursor
	Vector2d                      m_cursorPosition;

	///the name of the currently selected palette
	std::string                   m_selectedPalette;

	///is horizontal snap to grid on?
	bool                          m_snapToHorizontalGrid;
	///number of grid segments
	int                           m_snapToGridLinesPerHorizontalSide;
	///grid box size
	real                          m_snapToGridHorizontalSize;
	///m_snapToGridLinesPerHorizontalSide * m_snapToGridHorizontalSize, but precompute for speed
	real                          m_snapToGridHorizontalLineLength;
	///how easy it is to move from one grid segment to the next when moving objects
	int                           m_snapToGridHorizontalThreshold;

	///is vertical snap to grid on?
	bool                          m_snapToVerticalGrid;
	///number of grid segments
	int                           m_snapToGridLinesPerVerticalSide;
	///grid box size
	real                          m_snapToGridVerticalSize;
	///m_snapToGridLinesPerHorizontalSide * m_snapToGridHorizontalSize, but precompute for speed
	real                          m_snapToGridVerticalLineLength;
	///how easy it is to move from one grid segment to the next when moving objects
	int                           m_snapToGridVerticalThreshold;

	///whether someone has submitted a custom paste location
	bool                          m_pasteLocationKnown;
	///the custom-assigned paste location, if any
	Vector                        m_pasteLocation;

	//points that represent the camera-ground collision points
	Vector                        m_mouseCursorIntersection;
	Vector                        m_intersectionStart;
	Vector                        m_intersectionEnd;

	Vector                        m_pivotPoint;
	
	bool                          m_objectCreationPending;
	
	bool                          m_toggleDropToTerrainOn;
	bool                          m_toggleAlignToTerrainOn;

	ClipboardList_t               m_currentBrush;
};

// ----------------------------------------------------------------------

inline Vector GodClientData::getMouseCursorIntersection() const
{
	return m_mouseCursorIntersection;
}

// ======================================================================

#endif



