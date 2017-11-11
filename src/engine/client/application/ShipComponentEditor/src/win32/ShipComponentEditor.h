// ============================================================================
//
// ShipComponentEditor.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef ShipComponentEditor_h
#define ShipComponentEditor_h

#include "BaseShipComponentEditor.h"

class AudioDebugWidget;
class Object;
class SoundDebugInformationWidget;
class QWorkspace;

//-----------------------------------------------------------------------------
class ShipComponentEditor : public BaseShipComponentEditor
{
	Q_OBJECT //lint !e1924 !e1511 !e1516

public:

	ShipComponentEditor(QWidget *parent, char const *name);
	~ShipComponentEditor();

public slots:

	void onMenuReloadTemplates();
	void onMenuRegenerateTemplateDb();
	void onMenuSaveAll();
	void onMenuExit();
	void onMenuP4EditFiles();

	void installEngine();
	void alter();

signals:

protected:

	virtual void mouseReleaseEvent(QMouseEvent *mouseEvent);

private:

	ShipComponentEditor();

private:

	QWorkspace *                 m_workSpace;
	QPopupMenu *                 m_workspacePopUpMenu;
	Object *                     m_listener;
};

// ============================================================================

#endif
