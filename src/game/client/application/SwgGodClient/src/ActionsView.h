// ======================================================================
//
// ActionsView.h
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ActionsView_H
#define INCLUDED_ActionsView_H

// ======================================================================

#include "Singleton/Singleton.h"

// ======================================================================

class ActionHack;
class Vector;
class Object;

// ======================================================================

/**
* ActionsView is the resting place of various view-menu related actions
*/
class ActionsView : public QObject, public Singleton<ActionsView>, public MessageDispatch::Receiver
{
	Q_OBJECT; //lint !e1516 !e19 !e1924 !e1762 various deficiencies in the Qt macro

public:

	enum FitType
	{
		Fit_no,
		Fit_yes,
		Fit_point,
		Fit_dontCare
	};

	//lint -save
	//lint -e1925 // public data member
	ActionHack*       addCameraBookmark;
	ActionHack*       addObjectBookmark;
	ActionHack*       removeObjectBookmark;
	ActionHack*       centerSelection;
	ActionHack*       centerGhosts;
	ActionHack*       moveToCursor;
	ActionHack*       fitSelection;
	ActionHack*       fitGhosts;
	ActionHack*       showTriggerVolumes;
	ActionHack*       turnOffSpheres;
	ActionHack*       resetCamera;
	//lint -restore

	void receiveMessage(const MessageDispatch::Emitter& source, const MessageDispatch::MessageBase& message);

public:
	ActionsView();
	~ActionsView();

private slots:
	void               onAddCameraBookmark   () const;
	void               onAddObjectBookmark   () const;
	void               onRemoveObjectBookmark() const;
	void               onCenterSelection     () const;
	void               onCenterGhosts        () const;
	void               onMoveToCursor        () const;
	void               onFitSelection        () const;
	void               onFitGhosts           () const;
	void               onShowTriggerVolumes  () const;
	void               onTurnOffSpheres      () const;
	void               onResetCamera         () const;

public slots:
	void               centerCameraOnPoint (const Vector& pos, bool interpolate, int fit) const;
	void               centerCameraOnObject(const Object& obj, bool interpolate, int fit) const;
	void               gotoCameraBookmark  (int index) const;
	void               gotoObjectBookmark  (int index) const;

private:
	void               internalCenterSelection(bool ghosts, FitType fit) const;

private:
	//disabled
	ActionsView(const ActionsView& rhs);
	ActionsView& operator=(const ActionsView& rhs);
};

// ======================================================================

#endif
