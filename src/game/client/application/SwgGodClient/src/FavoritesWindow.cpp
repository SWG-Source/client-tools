// ======================================================================
//
// FavoritesBrowser.cpp
// copyright(c) 2005 Sony Online Entertainment
//
// ======================================================================

#include "SwgGodClient/FirstSwgGodClient.h"
#include "FavoritesWindow.h"
#include "FavoritesWindow.moc"
#include "FavoritesListView.h"
#include "IconLoader.h"
#include <qdom.h>
#include <qfile.h>
#include <qlistview.h>
#include <qmessagebox.h>
#include <qtoolbutton.h>

// ======================================================================
namespace
{
	//char const * const cs_FavoritesFile = "../../dev/win32/GodClientData/favorites.xml";	
	char const * const cs_FavoritesFile = "godclient_favorites.xml";
}

//-----------------------------------------------------------------

FavoritesWindow::FavoritesWindow(QWidget* theParent, const char* theName)
: BaseFavoritesWindow(theParent, theName)
{
	load();
}

//----------------------------------------------------------------------

FavoritesWindow::~FavoritesWindow()
{
	save();
}

//----------------------------------------------------------------------

std::string FavoritesWindow::getSelectedItemTemplateName()
{
	FavoritesListViewItem *currentItem = dynamic_cast<FavoritesListViewItem*> (m_favoritesList->currentItem());

	if(currentItem)
	{
		return currentItem->text(1).latin1();
	}
	return "";
}

//----------------------------------------------------------------------

void FavoritesWindow::addClientTemplateToRoot(const std::string& templateName)
{
	FavoritesListViewItem* rootListViewItem = dynamic_cast<FavoritesListViewItem*> ( m_favoritesList->firstChild() );
	if(rootListViewItem)
	{
		new FavoritesListViewItem(FavoritesListViewItem::CLIENT_TEMPLATE,rootListViewItem,"New Client Template",templateName.c_str());
	}

}

//----------------------------------------------------------------------

void FavoritesWindow::addServerTemplateToRoot(const std::string& templateName)
{
	FavoritesListViewItem* rootListViewItem = dynamic_cast<FavoritesListViewItem*> ( m_favoritesList->firstChild() );
	if(rootListViewItem)
	{
		new FavoritesListViewItem(FavoritesListViewItem::SERVER_TEMPLATE,rootListViewItem,"New Server Template",templateName.c_str());
	}
}

//----------------------------------------------------------------------
void FavoritesWindow::save()
{
	QFile opmlFile(cs_FavoritesFile);
	if(!opmlFile.open(IO_WriteOnly))
	{
		IGNORE_RETURN(QMessageBox::critical(0, QObject::tr("Critical Error"), QObject::tr("Cannot open file %1").arg(cs_FavoritesFile)));
		return;
	}

	QTextStream textOStream(&opmlFile);
	//-- output the header
	textOStream << "<?xml version=\"1.0\"?>" << endl;
	QDomDocument* document = new QDomDocument();
	QDomElement rootDomElement = document->createElement("GodClient_Favorites");
	rootDomElement.setAttribute("version", "1.0");

	FavoritesListViewItem* rootListViewItem = dynamic_cast<FavoritesListViewItem*> ( m_favoritesList->firstChild() );
	if(rootListViewItem)
	{
		convertListViewItem2QDomElement(document,rootListViewItem,&rootDomElement);
	}

	document->appendChild(rootDomElement);
	document->save(textOStream,4);
	opmlFile.close();

	delete document;
	document = NULL;
}
//----------------------------------------------------------------------
void FavoritesWindow::load()
{
	// create root folder
	m_favoritesList->clear();
	FavoritesListViewItem *rootListViewItem = new FavoritesListViewItem(
			FavoritesListViewItem::ROOT,m_favoritesList,"root"
		);
	rootListViewItem->setPixmap(0,IL_PIXMAP(hi16_action_bookmark_red_folder));
	rootListViewItem->setSelectable(false);

	QFile opmlFile(cs_FavoritesFile);
	
	if (!opmlFile.open(IO_ReadOnly))
	{
		DEBUG_WARNING(true,("FavoritesWindow::load() could not open %s",cs_FavoritesFile));
		return;
	}

	QDomDocument* document = new QDomDocument();
	QString errorMsg;
	int errorLine;
	int errorCol;
	if (!document->setContent(&opmlFile,&errorMsg,&errorLine,&errorCol))
	{
		IGNORE_RETURN(QMessageBox::critical(0, QObject::tr("Critical Error"), QObject::tr("Parsing error for file %1").arg(cs_FavoritesFile)));
		opmlFile.close();
		delete document;
		document = NULL;
		return;
	}	
	opmlFile.close();

// build up tree
	buildFavoritesList(document,rootListViewItem);
// end build up tree

	delete document;
	document = NULL;
}

//----------------------------------------------------------------------

void FavoritesWindow::buildFavoritesList(QDomNode* parentNode, FavoritesListViewItem* parentListViewItem)
{	
	FavoritesListViewItem *currentItem = parentListViewItem;
	QDomNode node = parentNode->firstChild();
	while(!node.isNull())
	{
		if(node.isElement())
		{
			QDomElement element = node.toElement();
			QString elementNameAttr = element.attribute("name");
			QString elementTypeAttr = element.attribute("type");
			QString elementValueAttr = element.attribute("value");
			if(elementTypeAttr == "FOLDER")
			{
				currentItem = new FavoritesListViewItem(
					FavoritesListViewItem::FOLDER,parentListViewItem,elementNameAttr
				);	
			}
			else if(elementTypeAttr == "SERVER_TEMPLATE")
			{
				currentItem = new FavoritesListViewItem(
					FavoritesListViewItem::SERVER_TEMPLATE,parentListViewItem,elementNameAttr,elementValueAttr
				);	
			}
			else if(elementTypeAttr == "CLIENT_TEMPLATE")
			{
				currentItem = new FavoritesListViewItem(
					FavoritesListViewItem::CLIENT_TEMPLATE,parentListViewItem,elementNameAttr,elementValueAttr
				);	
			}
		}
		buildFavoritesList(&node,currentItem);
		node = node.nextSibling();
	}
}

//----------------------------------------------------------------------

void FavoritesWindow::convertListViewItem2QDomElement(QDomDocument* document,FavoritesListViewItem* parentListViewItem,QDomElement* parentDomElement)
{
	FavoritesListViewItem* currentListViewItem = dynamic_cast<FavoritesListViewItem*> (parentListViewItem->firstChild());
	while(currentListViewItem)
	{
		QDomElement currentDomElement = document->createElement("Item");
		if(currentListViewItem->getType() == FavoritesListViewItem::FOLDER)
		{
			currentDomElement.setAttribute("type","FOLDER");
		}
		else if(currentListViewItem->getType() == FavoritesListViewItem::SERVER_TEMPLATE)
		{
			currentDomElement.setAttribute("type","SERVER_TEMPLATE");
			currentDomElement.setAttribute("value",currentListViewItem->text(1));
		}
		else if(currentListViewItem->getType() == FavoritesListViewItem::CLIENT_TEMPLATE)
		{
			currentDomElement.setAttribute("type","CLIENT_TEMPLATE");
			currentDomElement.setAttribute("value",currentListViewItem->text(1));
		}
		currentDomElement.setAttribute("name",currentListViewItem->text(0));
		convertListViewItem2QDomElement(document,currentListViewItem,&currentDomElement);
		parentDomElement->appendChild(currentDomElement);
		currentListViewItem = dynamic_cast<FavoritesListViewItem*> (currentListViewItem->nextSibling());
	}
}

// ======================================================================
