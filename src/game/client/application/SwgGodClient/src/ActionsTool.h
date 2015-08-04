// ======================================================================
//
// ActionsTool.h
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ActionsTool_H
#define INCLUDED_ActionsTool_H

// ======================================================================

#include "Singleton/Singleton.h"

// ======================================================================

class ActionHack;

//----------------------------------------------------------------------

/**
 */
class ActionsTool : public QObject, public Singleton<ActionsTool>
{
	Q_OBJECT;  //lint !e1516 !e19 !e1924 !e1762 various deficiencies in the Qt macro

public:
	//lint -save
	//lint -e1925 // public data member
	ActionHack*             m_snapToGrid;
	ActionHack*             m_saveAsBrush;
	ActionHack*             m_createBrush;
	ActionHack*             m_deleteBrush;
	ActionHack*             m_pasteBrushHere;
	ActionHack*             m_createPalette;
	ActionHack*             m_deletePalette;
	ActionHack*             m_getSphereTree;
	ActionHack*             m_grabRelativeCoordinates;
	ActionHack*             m_sendSystemMessage;
	//lint -restore

public:
	ActionsTool ();
	~ActionsTool();

private slots:
	void onSnapToGrid             () const;
	void onSaveAsBrush            () const;
	void onCreateBrush            () const;
	void onDeleteBrush            () const;
	void onPasteBrushHere         () const;
	void onCreatePalette          () const;
	void onDeletePalette          () const;
	void onShowSphereTree         () const;
	void onGrabRelativeCoordinates() const;
	void onSendSystemMessage      () const;

private:
	//disabled
	ActionsTool(const ActionsTool& rhs);
	ActionsTool& operator=(const ActionsTool& rhs);
};

// ======================================================================

#endif

