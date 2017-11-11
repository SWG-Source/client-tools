// ======================================================================
//
// ChannelWindow.h
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef ChannelWindow_H
#define ChannelWindow_H

#include "FirstRemoteDebugTool.h"

#include <qlistview.h>   //for the items derived off ListViewItem
#include <qsplitter.h> //base for our main class

#include <map>
#include <string>
#include <vector>

// -----------------------------------------------------------------

class QListView;
class QLabel;
class QPainter;
class QColorGroup;
class QObjectList;
class QPopupMenu;
class QTextView;
class QPixmap;

// -----------------------------------------------------------------

//declare our own inner classes, since definition order is tricky
class Node;
class ChannelNode;
class VariableNode;
class StaticViewNode;
class FolderListItem;

// -----------------------------------------------------------------

/** This class is the main window class of the application.  It 
  * is based off a splitter window, and loads a QListView into the
  * left pane, and a QTextView into the right pane.  It also 
  * manages all the context menus.
  */
class ChannelWindow : public QSplitter
{
	//necessary to make it a Qt object
	Q_OBJECT

	enum RemoteDebug::VARIABLE_TYPES;

public:
	explicit ChannelWindow( QWidget *parent, const char *name = NULL);
	virtual ~ChannelWindow();

	QTextView *getEditor() {return m_editorWindow;}

public:
	void newStream(uint32 streamNumber, const char *newChannelName);
	void messageOnStream(uint32 streamNumber, const char *message);
	void newVariable(uint32 variableNumber, const char *newVariableName);
	void variableType(uint32 variableNumber, RemoteDebug::VARIABLE_TYPES type);
	void variableValue(uint32 variableNumber, const char *value);
	void newStaticView(uint32 staticViewNumber, const char *newChannelName);
	void messageOnStaticView(uint32 staticViewNumber, const char *message);
	void beginFrame(uint32 channeNumber);
	void endFrame(uint32 channeNumber);
	void clearTree();
	void initFolders();

private:
	//clear out our local data
	void emptyDataStructures();

private:

	///icon for the tree view
	QPixmap        *m_displayedIcon;
	///icon for the tree view
	QPixmap        *m_notDisplayedIcon;
	///icon for the tree view
	QPixmap        *m_squelchedIcon;
	///icon for the tree view
	QPixmap        *m_closedIcon;
	///icon for the tree view
	QPixmap        *m_openedIcon;
	///icon for the tree view
	QPixmap        *m_inputTargetIcon;
	///The left pane of the app, holds a list view of channels, variables, etc.
	QListView      *m_folders;
	///The right pane of the app, a text view, nice fonts, etc.
	QTextView      *m_editorWindow;
	///PopupMenu for the channels
	QPopupMenu     *m_channelPopupMenu;
	///PopupMenu for the variables
	QPopupMenu     *m_variablesPopupMenu;
	///PopupMenu for the static views
	QPopupMenu     *m_staticViewPopupMenu;
	///The default font
	QFont           m_defaultFont;
	///conveinance pointer used to hold the most recently touched tree item
	FolderListItem *m_currentItem;

	ChannelNode    *m_channels;

	VariableNode   *m_variables;

	StaticViewNode *m_staticViews;


	///list of all the folders at the top level
	typedef std::map<Node *, FolderListItem *> NodeMap;
	NodeMap m_folderItems;
	typedef std::map<std::string, Node *> NodeNameMap;
	NodeNameMap m_folderItemNames;
	typedef std::map<uint32, std::string> ChannelNumberMap;
	ChannelNumberMap m_channelMap;
	typedef std::map<uint32, std::string> StaticViewNumberMap;
	StaticViewNumberMap m_staticViewMap;
	typedef std::map<uint32, std::string> VariableNumberMap;
	VariableNumberMap m_variableMap;
	typedef std::map<uint32, std::string> StaticViewFrameBufferMap;
	StaticViewFrameBufferMap m_staticViewFrameBufferMap;

	///This enum stores menu id's used at runtime to determine what item was pressed
	enum MENU_IDS
	{
		ID_CHANNEL_DISPLAY,
		ID_CHANNEL_SQUELCH,
		ID_CHANNEL_FONT,
		ID_CHANNEL_PROPERTIES,
		ID_CHANNEL_PRINT_TEST_STRING,
		ID_VARIABLE_SETVALUE,
		ID_VARIABLE_PROPERTIES,
		ID_STATICVIEW_UPDATE,
		ID_STATICVIEW_DISPLAY,
		ID_STATICVIEW_SQUELCH,
		ID_STATICVIEW_FONT,
		ID_STATICVIEW_PROPERTIES
	};

protected:
	void keyPressEvent( QKeyEvent * );

private slots:
	void slotFolderChanged(QListViewItem *);
	void slotFolderExpanded(QListViewItem *);
	void slotFolderCollapsed(QListViewItem *);
	void slotRMB(QListViewItem *, const QPoint &, int);
	//channel menu slots
	void slotChannelDisplay();
	void slotChannelSquelch();
	void slotChannelFont();
	void slotChannelProperties();
	//variable menu slots
	void slotVariableSet();
	void slotPropertiesChannel();
	//staticview menu slots
	void slotStaticViewUpdate();
	void slotStaticViewDisplay();
	void slotStaticViewSquelch();
	void slotStaticViewFont();
	void slotStaticViewProperties();

//disabled
private:
	ChannelWindow();
	ChannelWindow (const ChannelWindow&);
	ChannelWindow& operator= (const ChannelWindow&);
};

// -----------------------------------------------------------------

/** The base data object.  All the tree items hold one of these, and will
  * usually hold a more specific (i.e. derived version of) node type.  This
  * keeps the UI portion (specificially the FolderListItem) separate from 
  * the application specific data.
  */
class Node
{
public:
	///An enum used to store the derived node's type, useful for casting
	enum NODE_TYPE
	{
		CHANNEL,
		VARIABLE,
		STATIC_VIEW
	};

	Node( Node *parent, const std::string &name, NODE_TYPE type, const QFont &font, int channelNumber);
	virtual ~Node() {}

	//mutators
	void setFont(const QFont &f) {m_font = f;}

	bool displayed() {return m_displayed;}
	bool squelched() {return m_squelched;}
	bool inputTarget() {return m_inputTarget;}
	void setDisplayed(bool display) {m_displayed = display;}
	void setSquelched(bool squelch) {m_squelched = squelch;}
	void setInputTarget(bool target) {m_inputTarget = target;}
	int  channelNumber() {return m_channelNumber;}
	const std::string &getName() {return m_name;};

	//accessors
	QFont font() {return m_font;}
	std::string name() { return m_name; }
	NODE_TYPE type() {return m_node_type;}

private:
	//we don't want this, so declare it private and don't define it
	Node();

protected:
	///enum holding the node type, useful to determine how to cast the object
	NODE_TYPE m_node_type;
	///string used as the text label when displaying
	std::string m_name;
	///font used to display info sent to this channel
	QFont m_font;
	///all its children

protected:
	bool m_displayed;
	bool m_squelched;
	bool m_inputTarget;
	int  m_channelNumber;
};

// -----------------------------------------------------------------

class ChannelNode : public Node
{
public:
	ChannelNode(ChannelNode *parent, const std::string &name, const QFont &font, int channelNumber);
	virtual ~ChannelNode() {}

private:
	//we don't want this, so declare it private and don't define it
	ChannelNode();
};

// -----------------------------------------------------------------

class VariableNode : public Node
{
public:
	union VARIABLEVALUE
	{
		int32 intValue;
		float floatValue;
		int32 boolValue;
		char* stringValue;
	};
	VariableNode(VariableNode *parent, const std::string &name, const QFont &font, int channelNumber);
	virtual ~VariableNode() {}
	void setValue(void* newValue);
	void setType(RemoteDebug::VARIABLE_TYPES type);
	VARIABLEVALUE getValue();
	RemoteDebug::VARIABLE_TYPES getType();

private:
	RemoteDebug::VARIABLE_TYPES m_type;
	VARIABLEVALUE m_value;

	//we don't want this, so declare it private and don't define it
	VariableNode();
};

// -----------------------------------------------------------------

class StaticViewNode : public Node
{
public:
	StaticViewNode(StaticViewNode *parent, const std::string &name, const QFont &font, int channelNumber);
	virtual ~StaticViewNode() {}

private:
	//we don't want this, so declare it private and don't define it
	StaticViewNode();
};

// -----------------------------------------------------------------

/** This class is directly derived from a Qt list item.  It is mostly 
  * specialized so that it can hold a Node (app-specific datatype).
  */
class FolderListItem : public QListViewItem
{
public:
	FolderListItem(QListView *listViewParent, FolderListItem *parent, Node *n, const std::string &displayedName, QPixmap *pixmap = NULL);
	FolderListItem(FolderListItem *parent, Node *n, const std::string &displayedName, QPixmap *pixmap = NULL);

	Node *node() { return m_node; }
	void setDisplayedText(const std::string &s);
	const std::string &getDisplayedText() {return m_displayedName;}
	bool isLeaf() {return m_leaf;}
	void setNonLeaf() {m_leaf = false; stripValue();}

private:
	///we don't want this, so declare it private and don't define it
	FolderListItem();

	///remove the " = <value>" from the displayed string (used when a channel becomes a non-leaf node)
	void stripValue();

protected:
	///The attached data object, which also holds the fully qualified name for the item
	Node             *m_node;
	///The name of the item (the text displayed)
	std::string       m_displayedName;
	///Am I a leaf node
	bool              m_leaf;
};

// -----------------------------------------------------------------

#endif // ChannelWindow_H

