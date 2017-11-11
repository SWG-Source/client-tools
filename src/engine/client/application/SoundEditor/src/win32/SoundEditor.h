// ============================================================================
//
// SoundEditor.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef SOUNDEDITOR_H
#define SOUNDEDITOR_H

#include "BaseSoundEditor.h"

class AudioDebugWidget;
class Object;
class SoundDebugInformationWidget;
class QWorkspace;

//-----------------------------------------------------------------------------
class SoundEditor : public BaseSoundEditor
{
	Q_OBJECT

public:

	SoundEditor(QWidget *parent, char const *name);
	~SoundEditor();

public slots:

	void         alter();
	virtual void fileExit();
	virtual void fileNewSoundTemplateAction_activated();
	virtual void fileOpen();
	void         workspacePopUpMenuActivated(int index);
	void         slotFileCreateSpreadSheetActionActivated();

signals:

protected:

	virtual void mouseReleaseEvent(QMouseEvent *mouseEvent);

private:

	enum // WorkSpacePopupMenuOptions
	{
		WSPMO_newSoundTemplate,
		WSPMO_open
	};

	QWorkspace *                 m_workSpace;
	SoundDebugInformationWidget *m_debugInformationWidget;
	AudioDebugWidget *           m_audioDebugWidget;
	QPopupMenu *                 m_workspacePopUpMenu;
	Object *                     m_listener;
};

// ============================================================================

#endif // SOUNDEDITOR_H
