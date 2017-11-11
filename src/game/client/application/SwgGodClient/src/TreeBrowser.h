// ======================================================================
//
// TreeBrowser.h
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TreeBrowser_H
#define INCLUDED_TreeBrowser_H

// ======================================================================

#include "BaseTreeBrowser.h"
#include "ActionsView.h"

class QListViewItem;
class Object;

// ======================================================================

/**
* The TreeBrowser is a widget tabbed container, holding the Object browser(implemented here),
* The script browser(ScriptListView), and the template browser(TemplateListView)
*
* @todo: break the Object browser into its own class
* @todo: make the Object browser refresh properly when objects are added/removed from the world
*/

class TreeBrowser : public BaseTreeBrowser, public MessageDispatch::Receiver
{
	Q_OBJECT; //lint !e1516 !e19 !e1924 !e1762 various deficiencies in the Qt macro

public:
	explicit TreeBrowser(QWidget *theParent=0, const char *theName=0);
	virtual ~TreeBrowser();
	virtual void receiveMessage(const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);

public slots:
	void refreshObjects();

signals:
	void objectDoubleClickedSignal(const Object &, bool, int);
	void serverTemplateSelectionChanged(const std::string & name);
	void clientTemplateSelectionChanged(const std::string & name);

private slots:
	void onObjectSelectionChanged() const;
	void onObjectDoubleClicked(QListViewItem * item);
	void onServerTemplateListSelectionChanged();
	void onClientTemplateListSelectionChanged();

private:
	//disabled
	TreeBrowser & operator=(const TreeBrowser & rhs);
	TreeBrowser(const TreeBrowser & rhs);

private:
	void updateSelectionFromWorld();

private:
	QTimer*          m_refreshTimer;
	bool             m_updatingSelectionFromWorld;
};

// ======================================================================

#endif
