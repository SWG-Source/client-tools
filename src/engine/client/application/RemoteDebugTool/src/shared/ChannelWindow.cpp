// ======================================================================
//
// ChannelWindow.cpp
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "FirstRemoteDebugTool.h"
#include "ChannelWindow.h"
#include "ChannelWindow.moc"

#include "sharedDebug/RemoteDebug_inner.h"

#include <qlabel.h>
#include <qobjectlist.h>
#include <qpopupmenu.h>
#include <qinputdialog.h> 
#include <qheader.h>
#include <qregexp.h>
#include <qtextview.h>
#include <qpixmap.h>
#include <qfontdialog.h>

//include the bitmap files needed for the tree view
#include "res/channel_displayed.xpm"
#include "res/channel_not_displayed.xpm"
#include "res/channel_squelched.xpm"
#include "res/file_opened.xpm"
#include "res/file_closed.xpm"
#include "res/input_target.xpm"

// -----------------------------------------------------------------

Node::Node( Node *p, const std::string &n, NODE_TYPE t, const QFont &f, int c)
: m_node_type(t),
  m_name(n),
  m_font(f),
  m_displayed(true),
  m_squelched(false),
  m_inputTarget(false),
  m_channelNumber(c)
{
	UNREF(p);
}

// -----------------------------------------------------------------

ChannelNode::ChannelNode(ChannelNode *p, const std::string &n, const QFont &f, int c)
: Node(p, n, CHANNEL, f, c)
{}

// -----------------------------------------------------------------

VariableNode::VariableNode(VariableNode *p, const std::string &n, const QFont &f, int c)
: Node(p, n, VARIABLE, f, c),
  m_type(RemoteDebug::BOOL)
{}

// -----------------------------------------------------------------

void VariableNode::setType(RemoteDebug::VARIABLE_TYPES type)
{
	m_type = type;
}

// -----------------------------------------------------------------

void VariableNode::setValue(void* newValue)
{
	switch(m_type)
	{
		case RemoteDebug::INT:
			memcpy(&m_value.intValue, newValue, sizeof(int32));
			break;

		case RemoteDebug::FLOAT:
			memcpy(&m_value.floatValue, newValue, sizeof(float));
			break;

		case RemoteDebug::CSTRING:
			m_value.stringValue = new char[strlen(static_cast<char *>(newValue))];
			strcpy(m_value.stringValue, const_cast<const char *>(static_cast<char *>(newValue)));
			break;

		case RemoteDebug::BOOL:
			memcpy(&m_value.boolValue, newValue, sizeof(int32));
			break;
	}
}

// -----------------------------------------------------------------

RemoteDebug::VARIABLE_TYPES VariableNode::getType()
{
	return m_type;
}

// -----------------------------------------------------------------

VariableNode::VARIABLEVALUE VariableNode::getValue()
{
	return m_value;
}

// -----------------------------------------------------------------

StaticViewNode::StaticViewNode(StaticViewNode *p, const std::string &n, const QFont &f, int c)
: Node(p, n, STATIC_VIEW, f, c)
{
	m_squelched = true;
}

// -----------------------------------------------------------------

FolderListItem::FolderListItem(QListView *listViewParent, FolderListItem *p, Node *n, const std::string &displayedName, QPixmap *pixMap)
: QListViewItem( listViewParent, p),
  m_node(n),
  m_displayedName(displayedName),
	m_leaf(true)
{
	setText(0, m_displayedName.c_str());
	//set an image if given one
	if (pixMap)
		setPixmap(0, *pixMap);
	if(p)
		p->setNonLeaf();
}

// -----------------------------------------------------------------

FolderListItem::FolderListItem(FolderListItem *p, Node *n, const std::string &displayedName, QPixmap *pixMap)
: QListViewItem(p),
  m_node(n),
  m_displayedName(displayedName),
	m_leaf(true)
{
	setText(0, m_displayedName.c_str());
	//set an image if given one
	if (pixMap)
		setPixmap(0, *pixMap);
	if(p)
		p->setNonLeaf();
}

// -----------------------------------------------------------------

void FolderListItem::setDisplayedText(const std::string &s)
{
	m_displayedName = s;
	setText(0, m_displayedName.c_str());
}

// -----------------------------------------------------------------

//remove the " = <value>" from non-leaf nodes
void FolderListItem::stripValue()
{
	//eseses
	std::string text = m_displayedName;
	std::string token = " = ";
	size_t pos = text.rfind(token);
	if (pos != std::string::npos)
	{
		text.resize(pos);
	}
	setDisplayedText(text);
}

// -----------------------------------------------------------------

ChannelWindow::ChannelWindow(QWidget *p, const char *n)
: QSplitter( Qt::Horizontal, p, n),
  m_displayedIcon(NULL),
  m_notDisplayedIcon(NULL),
  m_squelchedIcon(NULL),
  m_closedIcon(NULL),
  m_openedIcon(NULL),
  m_inputTargetIcon(NULL),
  m_folders(NULL),
  m_editorWindow(NULL),
  m_channelPopupMenu(NULL),
  m_variablesPopupMenu(NULL),
  m_staticViewPopupMenu(NULL),
  m_defaultFont(),
  m_currentItem(NULL),
  m_channels(NULL),
  m_variables(NULL),
  m_staticViews(NULL),
  m_folderItems(),
  m_folderItemNames(),
  m_channelMap(),
  m_staticViewMap(),
  m_variableMap(),
  m_staticViewFrameBufferMap()
{
	//get the icons from the bitmap
	m_displayedIcon    = new QPixmap(channel_displayed);
	m_notDisplayedIcon = new QPixmap(channel_not_displayed);
	m_squelchedIcon    = new QPixmap(channel_squelched);
	m_closedIcon       = new QPixmap(file_closed);
	m_openedIcon       = new QPixmap(file_opened);
	m_inputTargetIcon  = new QPixmap(input_target);


	//create the left viw (a list view)
	m_folders = new QListView( this );
	m_folders->header()->setClickEnabled( FALSE );
	m_folders->addColumn( "Channels" );

	initFolders();

	m_folders->setRootIsDecorated( TRUE );
	setResizeMode( m_folders, QSplitter::KeepSize );

	//add the right view (a rich-edit like view)
	m_editorWindow = new QTextView( this, "editor" );
		m_editorWindow->setTextFormat(Qt::PlainText);

	m_defaultFont = m_editorWindow->font();

	//build the channel popup menu
	m_channelPopupMenu = new QPopupMenu(this);
	m_channelPopupMenu->insertItem("&Display",    this, SLOT( slotChannelDisplay()),    0, ID_CHANNEL_DISPLAY);
	m_channelPopupMenu->insertItem("&Squelch",    this, SLOT( slotChannelSquelch()),    0, ID_CHANNEL_SQUELCH);
	m_channelPopupMenu->insertItem("&Font",       this, SLOT( slotChannelFont()),       0, ID_CHANNEL_FONT);
	m_channelPopupMenu->insertItem("&Properties", this, SLOT( slotChannelProperties()), 0, ID_CHANNEL_PROPERTIES);

	//build the channel popup menu
	m_variablesPopupMenu = new QPopupMenu(this);
	m_variablesPopupMenu->insertItem("&Set Value",  this, SLOT( slotVariableSet()),       0, ID_VARIABLE_SETVALUE);
	m_variablesPopupMenu->insertItem("&Properties", this, SLOT( slotPropertiesChannel()), 0, ID_VARIABLE_PROPERTIES);

	//build the channel popup menu
	m_staticViewPopupMenu = new QPopupMenu(this);
	m_staticViewPopupMenu->insertItem("&Update",     this, SLOT( slotStaticViewUpdate()),     0, ID_STATICVIEW_UPDATE);
	m_staticViewPopupMenu->insertItem("&Display",    this, SLOT( slotStaticViewDisplay()),    0, ID_STATICVIEW_DISPLAY);
	m_staticViewPopupMenu->insertItem("&Squelch",    this, SLOT( slotStaticViewSquelch()),    0, ID_STATICVIEW_SQUELCH);
	m_staticViewPopupMenu->insertItem("&Font",       this, SLOT( slotStaticViewFont()),       0, ID_STATICVIEW_FONT);
	m_staticViewPopupMenu->insertItem("&Properties", this, SLOT( slotStaticViewProperties()), 0, ID_STATICVIEW_PROPERTIES);

	//get the keyboard events (so we can send input target events to the server)
	grabKeyboard();

	//connect the signals and slots

	//connect a signal for listview item changes
	connect(m_folders, SIGNAL(selectionChanged(QListViewItem *)),
	        this, SLOT(slotFolderChanged(QListViewItem *)));
	//connect a signal for listview item expansions
	connect(m_folders, SIGNAL(expanded(QListViewItem *)),
	        this, SLOT(slotFolderExpanded(QListViewItem *)));
	//connect a signal for listview item collapses
	connect(m_folders, SIGNAL(collapsed(QListViewItem *)),
	        this, SLOT(slotFolderCollapsed(QListViewItem *)));
	//connect a signal for listview item right clicks
	connect(m_folders, SIGNAL(rightButtonClicked(QListViewItem *, const QPoint &, int)),
	        this, SLOT(slotRMB(QListViewItem *, const QPoint &, int)));
}

// -----------------------------------------------------------------

ChannelWindow::~ChannelWindow()
{
	emptyDataStructures();

	if (m_displayedIcon)
		delete m_displayedIcon;
	if (m_notDisplayedIcon)
		delete m_notDisplayedIcon;
	if (m_squelchedIcon)
		delete m_squelchedIcon;
	if (m_closedIcon)
		delete m_closedIcon;
	if (m_openedIcon)
		delete m_openedIcon;
	if (m_inputTargetIcon)
		delete m_inputTargetIcon;

	//release Qt objects (which Qt *should* release itself)
	if(m_folders)
		delete m_folders;
	if(m_editorWindow)
		delete m_editorWindow;
	if(m_channelPopupMenu)
		delete m_channelPopupMenu;
	if(m_variablesPopupMenu)
		delete m_variablesPopupMenu;
	if(m_staticViewPopupMenu)
		delete m_staticViewPopupMenu;
/*
	if(m_currentItem)
		delete m_currentItem;
	if(m_channels)
		delete m_channels;
	if(m_variables)
		delete m_variables;
	if(m_staticViews)
		delete m_staticViews;
*/
}

// -----------------------------------------------------------------

void ChannelWindow::initFolders()
{
	emptyDataStructures();
	clearTree();

	//since it really isn't a channel ,set the channel number to -1 
	//(should probably make this a different kind of node)
	m_channels = new ChannelNode( 0, "Streaming Views", m_defaultFont , -1);
	FolderListItem *f = new FolderListItem(m_folders, static_cast<FolderListItem *>(m_folders->firstChild()), m_channels, "Streaming Views", m_closedIcon);
	m_folderItems[m_channels] = f;
	m_folderItemNames["Streaming Views"] = m_channels;

	m_staticViews = new StaticViewNode( 0, "Static Views", m_defaultFont, -1);
	f = new FolderListItem(m_folders, static_cast<FolderListItem *>(m_folders->firstChild()), m_staticViews, "Static Views", m_closedIcon);
	m_folderItems[m_staticViews] = f;
	m_folderItemNames["Static Views"] = m_staticViews;

	m_variables = new VariableNode( 0, "Variables", m_defaultFont, -1);
	f = new FolderListItem(m_folders, static_cast<FolderListItem *>(m_folders->firstChild()), m_variables, "Variables", m_closedIcon);
	m_folderItems[m_variables] = f;
	m_folderItemNames["Variables"] = m_variables;
}

// -----------------------------------------------------------------

void ChannelWindow::emptyDataStructures()
{
	m_folderItems.clear();

	for(NodeNameMap::iterator itr = m_folderItemNames.begin(); itr != m_folderItemNames.end(); ++itr)
	{
		delete itr->second;
	}
	m_folderItemNames.clear();

	m_channelMap.clear();

	m_staticViewMap.clear();

	m_variableMap.clear();

	m_staticViewFrameBufferMap.clear();
}

// -----------------------------------------------------------------

void ChannelWindow::clearTree()
{
	m_folders->clear();
}

// -----------------------------------------------------------------

void ChannelWindow::newStream(uint32 streamNumber, const char *newChannelName)
{
	//new channel is a child of the "channel" item if not a sub channel
	ChannelNode *p = m_channels;
	//pointer to the channel's name (we'll increment it past any base channels)
	const char *channelName = newChannelName;
	const char *baseBegin = newChannelName;
	const char *baseEnd = strchr(newChannelName, '\\');
	//split out the subchannel from the parent(s)
	while (baseEnd != NULL)
	{
		int baseLen = baseEnd - baseBegin;
		//get the base channel
		char *baseChannel = new char[static_cast<uint32>(baseLen+1)];
		memcpy(baseChannel, baseBegin, static_cast<uint32>(baseLen));
		baseChannel[baseLen] = '\0';
		//find the base channel
		p = static_cast<ChannelNode *>(m_folderItemNames[baseChannel]);
		delete[] baseChannel;
		channelName = baseEnd+1;
		baseBegin = baseEnd+1;
		baseEnd = strchr(baseEnd+1, '\\');
	}
	//create a new node
	ChannelNode *n = new ChannelNode(p, newChannelName, m_defaultFont, static_cast<int32>(streamNumber));
	//add it to the treeview
	FolderListItem *channelsItem = m_folderItems[p];
	FolderListItem *f = new FolderListItem(channelsItem, n, channelName, m_displayedIcon);
	m_folders->ensureItemVisible(f);
	m_folderItems[n] = f;
	m_channelMap[streamNumber] = channelName;
	m_folderItemNames[channelName] = n;
}

// -----------------------------------------------------------------

void ChannelWindow::messageOnStream(uint32 streamNumber, const char *message)
{
	std::string s = m_channelMap[streamNumber];
	ChannelNode *n = static_cast<ChannelNode *>(m_folderItemNames[s]);
	if (n && n->displayed() && *message)
	{
		m_editorWindow->setFont(n->font());
		m_editorWindow->append(message);
		QScrollBar *verticalScrollBar = m_editorWindow->verticalScrollBar();
		int maxRange = verticalScrollBar->maxValue();
		verticalScrollBar->setValue(maxRange);
	}
}

// -----------------------------------------------------------------

void ChannelWindow::newStaticView(uint32 staticViewNumber, const char *newChannelName)
{
	//new channel is a child of the "channel" item if not a sub channel
	StaticViewNode *p = m_staticViews;
	//pointer to the channel's name (we'll increment it past any base channels)
	const char *channelName = newChannelName;
	const char *baseBegin = newChannelName;
	const char *baseEnd = strchr(newChannelName, '\\');
	//split out the subchannel from the parent(s)
	while (baseEnd != NULL)
	{
		int baseLen = baseEnd - baseBegin;
		//get the base channel
		char *baseChannel = new char[static_cast<uint32>(baseLen+1)];
		memcpy(baseChannel, baseBegin, static_cast<uint32>(baseLen));
		baseChannel[baseLen] = '\0';
		//find the base channel
		p = static_cast<StaticViewNode *>(m_folderItemNames[baseChannel]);
		delete[] baseChannel;
		channelName = baseEnd+1;
		baseBegin = baseEnd+1;
		baseEnd = strchr(baseEnd+1, '\\');
	}
	//create a new node
	StaticViewNode *n = new StaticViewNode(p, newChannelName, m_defaultFont, static_cast<int32>(staticViewNumber));
	//add it to the treeview
	FolderListItem *channelsItem = m_folderItems[p];
	FolderListItem *f = new FolderListItem(channelsItem, n, channelName, m_squelchedIcon);
	m_folders->ensureItemVisible(f);
	m_folderItems[n] = f;
	m_staticViewMap[staticViewNumber] = channelName;
	m_folderItemNames[channelName] = n;

	m_staticViewFrameBufferMap[staticViewNumber] = "";
	m_staticViewFrameBufferMap[staticViewNumber].reserve(4096);
}

// -----------------------------------------------------------------

void ChannelWindow::messageOnStaticView(uint32 staticViewNumber, const char *message)
{
	std::string s = m_staticViewMap[staticViewNumber];
	StaticViewNode *n = static_cast<StaticViewNode *>(m_folderItemNames[s]);
	if (n && n->displayed() && *message)
	{
		m_staticViewFrameBufferMap[staticViewNumber] += message;
	}
}
// -----------------------------------------------------------------

void ChannelWindow::beginFrame(uint32 staticViewNumber)
{
	UNREF(staticViewNumber);
}

// -----------------------------------------------------------------

void ChannelWindow::endFrame(uint32 staticViewNumber)
{
	std::string s = m_staticViewMap[staticViewNumber];
	StaticViewNode *n = static_cast<StaticViewNode *>(m_folderItemNames[s]);
	if (n && n->displayed())
	{
		m_editorWindow->setFont(n->font());
		//print the new frame
		m_editorWindow->setText(m_staticViewFrameBufferMap[staticViewNumber].c_str());
		//now clear the frame buffer
		m_staticViewFrameBufferMap[staticViewNumber] = "";
	}
}

// -----------------------------------------------------------------

void ChannelWindow::newVariable(uint32 variableNumber, const char *newVariableName)
{
	//new channel is a child of the "channel" item if not a sub channel
	VariableNode *p = m_variables;
	//pointer to the channel's name (we'll increment it past any base channels)
	const char *variableName = newVariableName;
	const char *baseBegin = variableName;
	const char *baseEnd = strchr(variableName, '\\');
	//split out the subchannel from the parent(s)
	while (baseEnd != NULL)
	{
		int baseLen = baseEnd - baseBegin;
		//get the base channel
		char *baseChannel = new char[static_cast<uint32>(baseLen+1)];
		memcpy(baseChannel, baseBegin, static_cast<uint32>(baseLen));
		baseChannel[baseLen] = '\0';
		//find the base channel
		p = static_cast<VariableNode *>(m_folderItemNames[baseChannel]);
		delete[] baseChannel;
		variableName = baseEnd+1;
		baseBegin = baseEnd+1;
		baseEnd = strchr(baseEnd+1, '\\');
	}
	//create a new node
	VariableNode *n = new VariableNode(p, newVariableName, m_defaultFont, static_cast<int>(variableNumber));
	//add it to the treeview
	FolderListItem *channelsItem = m_folderItems[p];
	FolderListItem *f = new FolderListItem(channelsItem, n, variableName, m_displayedIcon);
	m_folders->ensureItemVisible(f);
	m_folderItems[n] = f;
	m_variableMap[variableNumber] = variableName;
	m_folderItemNames[variableName] = n;
}

// -----------------------------------------------------------------

void ChannelWindow::variableType(uint32 variableNumber, RemoteDebug::VARIABLE_TYPES type)
{
	std::string s = m_variableMap[variableNumber];
	VariableNode *n = static_cast<VariableNode *>(m_folderItemNames[s]);
	if (n)
	{
		n->setType(type);
	}
}

// -----------------------------------------------------------------

void ChannelWindow::variableValue(uint32 variableNumber, const char *varValue)
{
	std::string s = m_variableMap[variableNumber];
	VariableNode *n = static_cast<VariableNode *>(m_folderItemNames[s]);
	if (n)
	{
		n->setValue(const_cast<char *>(varValue));
		FolderListItem *f = m_folderItems[n];
		std::string text = f->getDisplayedText();
		std::string token = " = ";
		size_t pos = text.rfind(token);
		if (pos != std::string::npos)
		{
			text.resize(pos + token.size());
		}
		else
			text += token;
		char buffer[256];
		VariableNode::VARIABLEVALUE v = n->getValue();
		switch(n->getType())
		{
			case RemoteDebug::BOOL:
				if (v.boolValue == 0)
					text += "false";
				else
					text += "true";
				break;

			case RemoteDebug::FLOAT:
				gcvt(v.floatValue, 10, buffer);
				text += buffer;
				break;
			
			case RemoteDebug::INT:
				_itoa(v.intValue, buffer, 10);
				text += buffer;
				break;
			
			case RemoteDebug::CSTRING:
				text += v.stringValue;
				break;
		}
		//only display values on leaves
		if(f->isLeaf() == true)
			f->setDisplayedText(text);
	}
}

// -----------------------------------------------------------------

void ChannelWindow::keyPressEvent( QKeyEvent * k)
{
	switch (k->key())
	{
		case Key_Up:
			RemoteDebugClient::send(RemoteDebug::STATIC_UP, "");
			break;

		case Key_Down:
			RemoteDebugClient::send(RemoteDebug::STATIC_DOWN, "");
			break;

		case Key_Left:
			RemoteDebugClient::send(RemoteDebug::STATIC_LEFT, "");
			break;

		case Key_Right:
			RemoteDebugClient::send(RemoteDebug::STATIC_RIGHT, "");
			break;

		case Key_Enter:
		case Key_Return:
			RemoteDebugClient::send(RemoteDebug::STATIC_ENTER, "");
			break;

		default:
			break;
	}
}

//end regular member functions
// -----------------------------------------------------------------
//Begin slots

/** This slot is called when any tree item gets right clicked.  We have to 
  * determine what menu to actually show, based on the item's node's type.
  */
void ChannelWindow::slotRMB( QListViewItem *item, const QPoint & point, int i)
{
	UNREF(i);
	FolderListItem *folderItem = static_cast<FolderListItem *>(item);
	if (!folderItem)
		return;

	//only act on leaves
	if(folderItem->isLeaf() == false)
		return;

	m_currentItem = folderItem;
	Node *node = folderItem->node();
	if (!node)
		return;
	Node::NODE_TYPE type = node->type();
	//determine node type, display correct menu
	if (type == Node::CHANNEL)
	{
		ChannelNode *channelNode = static_cast<ChannelNode *>(node);
		if (channelNode->squelched())
		{
			//if squelched, disable the display and font items
			m_channelPopupMenu->setItemChecked(ID_CHANNEL_SQUELCH, true);
			m_channelPopupMenu->setItemEnabled(ID_CHANNEL_DISPLAY, false);
			m_channelPopupMenu->setItemEnabled(ID_CHANNEL_FONT,    false);
		}
		else
		{
			//if not squelched, enable the display and font items
			m_channelPopupMenu->setItemChecked(ID_CHANNEL_SQUELCH, false);
			m_channelPopupMenu->setItemEnabled(ID_CHANNEL_DISPLAY, true);
			m_channelPopupMenu->setItemEnabled(ID_CHANNEL_FONT,    true);
		}
		if (channelNode->displayed())
		{
			m_channelPopupMenu->setItemChecked(ID_CHANNEL_DISPLAY, true);
		}
		else
		{
			m_channelPopupMenu->setItemChecked(ID_CHANNEL_DISPLAY, false);
		}
		m_channelPopupMenu->popup( point );
	}
	else if (type  == Node::VARIABLE)
	{
		m_variablesPopupMenu->popup( point );
	}
	else if (type  == Node::STATIC_VIEW)
	{
		StaticViewNode *staticViewNode = static_cast<StaticViewNode *>(node);
		if (staticViewNode->squelched())
		{
			//if squelched, disable the display and font items
			m_staticViewPopupMenu->setItemChecked(ID_STATICVIEW_SQUELCH, true);
			m_staticViewPopupMenu->setItemEnabled(ID_STATICVIEW_DISPLAY, false);
			m_staticViewPopupMenu->setItemEnabled(ID_STATICVIEW_FONT,    false);
		}
		else
		{
			//if not squelched, enable the display and font items
			m_staticViewPopupMenu->setItemChecked(ID_STATICVIEW_SQUELCH, false);
			m_staticViewPopupMenu->setItemEnabled(ID_STATICVIEW_DISPLAY, true);
			m_staticViewPopupMenu->setItemEnabled(ID_STATICVIEW_FONT,    true);
		}
		if (staticViewNode->displayed())
		{
			m_staticViewPopupMenu->setItemChecked(ID_STATICVIEW_DISPLAY, true);
		}
		else
		{
			m_staticViewPopupMenu->setItemChecked(ID_STATICVIEW_DISPLAY, false);
		}
		m_staticViewPopupMenu->popup( point );
	}
}

// -----------------------------------------------------------------

void ChannelWindow::slotFolderChanged(QListViewItem *item)
{
	UNREF(item);
}

// -----------------------------------------------------------------

void ChannelWindow::slotFolderExpanded(QListViewItem *item)
{
	FolderListItem *folderItem = static_cast<FolderListItem *>(item);
	folderItem->setPixmap(0, *m_openedIcon);
}

// -----------------------------------------------------------------

void ChannelWindow::slotFolderCollapsed(QListViewItem *item)
{
	FolderListItem *folderItem = static_cast<FolderListItem *>(item);
	folderItem->setPixmap(0, *m_closedIcon);
}

// -----------------------------------------------------------------

void ChannelWindow::slotChannelDisplay()
{
	//get item
	if (m_currentItem)
	{
		ChannelNode *node;
		if (m_currentItem->node()->type() == Node::CHANNEL)
			node = static_cast<ChannelNode *>(m_currentItem->node());
		else
			return;
		if (node)
		{
			bool newDisplayed = !node->displayed();
			//toggle displayed in item
			node->setDisplayed(newDisplayed);
			//change icon
			if(newDisplayed)
				m_currentItem->setPixmap(0, *m_displayedIcon);
			else
				m_currentItem->setPixmap(0, *m_notDisplayedIcon);
		}
	}
}

// -----------------------------------------------------------------

void ChannelWindow::slotChannelSquelch()
{
	//get item
	if (m_currentItem)
	{
		ChannelNode *node;
		if (m_currentItem->node()->type() == Node::CHANNEL)
			node = static_cast<ChannelNode *>(m_currentItem->node());
		else
			return;
		if (node)
		{
			bool newSquelched = !node->squelched();
			//toggle squelched in item
			node->setSquelched(newSquelched);
			//change icon
			if(newSquelched)
			{
				m_currentItem->setPixmap(0, *m_squelchedIcon);
				RemoteDebugClient::send(RemoteDebug::STREAM_SQUELCH, node->name().c_str());
			}
			else
			{
				//unsquelching, return icon to previous display state
				if(node->displayed())
					m_currentItem->setPixmap(0, *m_displayedIcon);
				else
					m_currentItem->setPixmap(0, *m_notDisplayedIcon);
				RemoteDebugClient::send(RemoteDebug::STREAM_UNSQUELCH, node->name().c_str());
			}
		}
	}
}

// -----------------------------------------------------------------

void ChannelWindow::slotChannelFont()
{
	//show font dialog
	bool result = false;
//	QFont newFont = QFontDialog::getFont(&result, m_defaultFont, this);
	QFont newFont = QFontDialog::getFont(&result, QFont( "Times", 12 ), this);
	if (result)
	{
		//store new font in the channel
		if (m_currentItem)
			m_currentItem->node()->setFont(newFont);
	}
}

// -----------------------------------------------------------------

void ChannelWindow::slotChannelProperties()
{
	//?
}

// -----------------------------------------------------------------


void ChannelWindow::slotVariableSet()
{
	if (m_currentItem)
	{
		VariableNode *node;
		if (m_currentItem->node()->type() == Node::VARIABLE)
			node = static_cast<VariableNode *>(m_currentItem->node());
		else
			return;
		if (node)
		{
			int32 intValue = 0;
			float floatValue = 0;
			QString textValue;
			int32 boolValue = 0;
			bool ok = false;
			
			bool haveValidValue = false;
			bool wantToQuit = false;
			//release the keyboard (normally grabbed for input events), so dialogs can accept text
			releaseKeyboard();
			while (!haveValidValue && !wantToQuit)
			{
				switch(node->getType())
				{
					case RemoteDebug::BOOL:
						textValue = QInputDialog::getText("Set Variable", "Enter the new bool value",  QLineEdit::Normal, QString::null, &ok, this);
						if (ok && !textValue.isEmpty())
						{
							textValue.lower();
							if (textValue == "true" || textValue == "t"|| textValue == "1")
							{
								boolValue = 1;
								variableValue(node->channelNumber(), reinterpret_cast<char *>(&boolValue));
								RemoteDebug::setVariableValue(node->name().c_str(), reinterpret_cast<char *>(&boolValue), true);
								haveValidValue = true;
							}
							else if (textValue == "false" || textValue == "f"|| textValue == "0")
							{
								boolValue = 0;
								variableValue(node->channelNumber(), reinterpret_cast<char *>(&boolValue));
								RemoteDebug::setVariableValue(node->name().c_str(), reinterpret_cast<char *>(&boolValue), true);
								haveValidValue = true;
							}
							else
								haveValidValue = false;
						}
						else
							wantToQuit = true;
						break;
					
					case RemoteDebug::INT:
						intValue = QInputDialog::getInteger("Set Variable", "Enter the new integer value", 0, -2147483647, 2147483647, 1, &ok, this);
						if (ok)
						{
							variableValue(node->channelNumber(), reinterpret_cast<char *>(&intValue));
							RemoteDebug::setVariableValue(node->name().c_str(), reinterpret_cast<char *>(&intValue), true);
							haveValidValue = true;
						}
						else
							wantToQuit = true;
						break;
					
					case RemoteDebug::FLOAT:
						floatValue = QInputDialog::getDouble("Set Variable", "Enter the new float value", 0, -2147483647, 2147483647, 1, &ok, this);
						if (ok)
						{
							variableValue(node->channelNumber(), reinterpret_cast<char *>(&floatValue));
							RemoteDebug::setVariableValue(node->name().c_str(), reinterpret_cast<char *>(&floatValue), true);
							haveValidValue = true;
						}
						else
							wantToQuit = true;
						break;
					
					case RemoteDebug::CSTRING:
						textValue = QInputDialog::getText("Set Variable", "Enter the new string value", QLineEdit::Normal, QString::null, &ok, this);
						if (ok && !textValue.isEmpty())
						{
							variableValue(node->channelNumber(), const_cast<char *>(textValue.latin1()));
							RemoteDebug::setVariableValue(node->name().c_str(), const_cast<char *>(textValue.latin1()), true);
							haveValidValue = true;
						}
						else
							wantToQuit = true;
						break;
				}
			}
			//reacquire the keyboard (so we can send input events directly to the server)
			grabKeyboard();
		}
	}
}

// -----------------------------------------------------------------

void ChannelWindow::slotPropertiesChannel()
{
	//?
}

// -----------------------------------------------------------------

void ChannelWindow::slotStaticViewUpdate()
{
	//force update of view (poll server for new data)
}

// -----------------------------------------------------------------

void ChannelWindow::slotStaticViewSquelch()
{
	//get item
	if (m_currentItem)
	{
		StaticViewNode *node;
		if (m_currentItem->node()->type() == Node::STATIC_VIEW)
			node = static_cast<StaticViewNode *>(m_currentItem->node());
		else
			return;
		if (node)
		{
			bool newSquelched = !node->squelched();
			//toggle squelched in item
			node->setSquelched(newSquelched);
			//change icon
			if(newSquelched)
			{
				m_currentItem->setPixmap(0, *m_squelchedIcon);
				RemoteDebugClient::send(RemoteDebug::STATIC_SQUELCH, node->name().c_str());
			}
			else
			{
				//unsquelching, return icon to previous display state
				if(node->displayed()) 
					m_currentItem->setPixmap(0, *m_displayedIcon);
				else
					m_currentItem->setPixmap(0, *m_notDisplayedIcon);
				RemoteDebugClient::send(RemoteDebug::STATIC_UNSQUELCH, node->name().c_str());
				//TODO FIXME move this elsewhere
				RemoteDebugClient::send(RemoteDebug::STATIC_INPUT_TARGET, node->name().c_str());
			}
		}
	}
}

// -----------------------------------------------------------------

void ChannelWindow::slotStaticViewDisplay()
{
	//get item
	if (m_currentItem)
	{
		StaticViewNode *node;
		if (m_currentItem->node()->type() == Node::STATIC_VIEW)
			node = static_cast<StaticViewNode *>(m_currentItem->node());
		else
			return;
		if (node)
		{
			bool newDisplayed = !node->displayed();
			//toggle displayed in item
			node->setDisplayed(newDisplayed);
			//change icon
			if(newDisplayed)
				m_currentItem->setPixmap(0, *m_displayedIcon);
			else
				m_currentItem->setPixmap(0, *m_notDisplayedIcon);
		}
	}
}

// -----------------------------------------------------------------

void ChannelWindow::slotStaticViewFont()
{
	//show font dialog
	bool result = false;
//	QFont newFont = QFontDialog::getFont(&result, m_defaultFont, this);
	QFont newFont = QFontDialog::getFont(&result, QFont( "Times", 12 ), this);
	if (result)
	{
		//store new font in the channel
		if (m_currentItem)
			m_currentItem->node()->setFont(newFont);
	}
}

// -----------------------------------------------------------------

void ChannelWindow::slotStaticViewProperties()
{
	//?
}

// -----------------------------------------------------------------
