// ======================================================================
//
// FavoritesWindow.h
// copyright(c) 2005 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_FavoritesWindow_H
#define INCLUDED_FavoritesWindow_H

// ======================================================================
#include "BaseFavoritesWindow.h"
#include <qlistview.h>
#include <qdom.h>

//-----------------------------------------------------------------

class QListViewItem;
class FavoritesListViewItem;

//-----------------------------------------------------------------

class FavoritesWindow : public BaseFavoritesWindow
{
	Q_OBJECT; 
public:
	FavoritesWindow(QWidget* parent, const char* name);
	~FavoritesWindow();

	std::string getSelectedItemTemplateName();
	void addClientTemplateToRoot(const std::string& templateName);
	void addServerTemplateToRoot(const std::string& templateName);
private slots:

signals:

private:
	//disabled
	FavoritesWindow();
	FavoritesWindow(const FavoritesWindow& rhs);
	FavoritesWindow & operator=(const FavoritesWindow& rhs);

	void save();
	void load();
	void convertListViewItem2QDomElement(QDomDocument* document,FavoritesListViewItem* parentListViewItem,QDomElement* parentDomElement);
	void buildFavoritesList(QDomNode* parentNode, FavoritesListViewItem* parentListViewItem);

};

// ======================================================================

#endif
