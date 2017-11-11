// ======================================================================
//
// ActionsEdit.h
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ActionsEdit_H
#define INCLUDED_ActionsEdit_H

// ======================================================================

#include "Singleton/Singleton.h"

#include "GodClientData.h" //needed for inner class

// ======================================================================

class ActionHack;
class QActionGroup;

// ======================================================================
/**
* ActionsEdit is the resting place of various edit-menu related actions
*/
class ActionsEdit : public QObject, public Singleton<ActionsEdit>, public MessageDispatch::Receiver
{
	Q_OBJECT; //lint !e1516 !e19 !e1924 !e1762 various deficiencies in the Qt macro

public:

	struct DragMessages
	{
		static const char* const PALETTE_DRAGGED_INTO_GAME;
		static const char* const PALETTE_DRAGGED_ONTO_PALETTE_WIDGET;
	};

	//lint -save
	//lint -e1925 // public data member
	ActionHack*             undo;
	ActionHack*             redo;
	ActionHack*             copy;
	ActionHack*             paste;
	ActionHack*             pasteBrush;
	ActionHack*             del;
	ActionHack*             cut;
	ActionHack*             createObjectFromSelectedTemplate;
	ActionHack*             dropToTerrain;
	ActionHack*             randomRotate;
	ActionHack*             applyTransform;
	ActionHack*             unlockSelected;
	ActionHack*             unlockAll;
	ActionHack*             unlockNonStructures;
	ActionHack*             unlockServerOnly;
	ActionHack*             alignToTerrain;
	ActionHack*             rotateReset;
	QActionGroup*           rotateModeGroup;
	ActionHack*             rotateModeYaw;
	ActionHack*             rotateModePitch;
	ActionHack*             rotateModeRoll;
	QActionGroup*           rotatePivotModeGroup;
	ActionHack*             rotatePivotModeSelf;
	ActionHack*             rotatePivotModeCenter;
	ActionHack*             rotatePivotModeLastSelected;
	ActionHack*             rotateIncrement45;
	ActionHack*             rotateDecrement45;
	ActionHack*             selectionStoreF1;
	ActionHack*             selectionStoreF2;
	ActionHack*             selectionStoreF3;
	ActionHack*             selectionStoreF4;
	ActionHack*             selectionRestoreF1;
	ActionHack*             selectionRestoreF2;
	ActionHack*             selectionRestoreF3;
	ActionHack*             selectionRestoreF4;
	ActionHack*             setTransform;
	ActionHack*             editTriggers;
	ActionHack*             editFormData;
	ActionHack*             copyForPOI;
	ActionHack*             showObjectAxes;
	ActionHack*             togglePauseParticleSystems;
	ActionHack*             createTheater;
	ActionHack*             createClientDataFile;
	ActionHack*				m_toggleDropToTerrain;
	ActionHack*				m_toggleAlignToTerrain;
	//lint -restore

	void receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);

private slots:
	void                  onUndo             () const;
	void                  onRedo             () const;
	void                  onDelete           () const;
	void                  onCopy             () const;
	void                  onPaste            () const;
	void                  onPasteBrush       () const;
	void                  onCut              () const;
	void                  onApplyTransform   () const;
	void                  onUnlockSelected   () const;
	void                  onUnlockAll        () const;
	void                  onUnlockNonStructures() const;
	void                  onUnlockServerOnly () const;
	void                  onDropToTerrain    () const;
	void                  onRandomRotate     () const;
	void                  onAlignToTerrain   () const;
	void                  onRotateReset      () const;
	void                  onRotateIncrement45() const;
	void                  onRotateDecrement45() const;
	void                  onStoreGroup1      () const;
	void                  onRestoreGroup1    () const;
	void                  onStoreGroup2      () const;
	void                  onRestoreGroup2    () const;
	void                  onStoreGroup3      () const;
	void                  onRestoreGroup3    () const;
	void                  onStoreGroup4      () const;
	void                  onRestoreGroup4    () const;
	void                  onSetTransform     () const;
	void                  onEditTriggers     () const;
	void                  onEditFormData     () const;
	void                  onCopyForPOI       () const;
	void                  onShowObjectAxes   () const;
	void                  onToggleParticleSystemPause() const;
	void                  onCreateTheater () const;
	void                  onCreateClientDataFile () const;
	void                  onToggleDropToTerrain () const;
	void                  onToggleAlignToTerrain () const;

public slots:
	void                  onServerTemplateSelectionChanged(const std::string &);
	void                  onClientTemplateSelectionChanged(const std::string &);

public:
	ActionsEdit();
	~ActionsEdit();
	const std::string & getSelectedServerTemplate() const;
	const std::string & getSelectedClientTemplate() const;
	int getRotatePivotMode() const;

private:
	//disabled
	ActionsEdit(const ActionsEdit & rhs);
	ActionsEdit & operator=(const ActionsEdit & rhs);

private:
	void                  internalDeleteSelection(bool confirm)  const;
	void                  internalRotateDiscrete(const real rad) const;
	void                  setSelectionControlsEnabled(bool b);
	void                  internalPaste(GodClientData::ClipboardList_t& clip) const;

private:
	std::string           m_selectedServerTemplate;
	std::string           m_selectedClientTemplate;
};
//-----------------------------------------------------------------

inline const std::string & ActionsEdit::getSelectedServerTemplate() const
{
	return m_selectedServerTemplate;
}

//-----------------------------------------------------------------

inline const std::string & ActionsEdit::getSelectedClientTemplate() const
{
	return m_selectedClientTemplate;
}

// ======================================================================

#endif
