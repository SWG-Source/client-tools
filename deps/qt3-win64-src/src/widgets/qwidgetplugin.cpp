/****************************************************************************
** $Id: qwidgetplugin.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of QWidgetPlugin class
**
** Created : 010920
**
** Copyright (C) 2001-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech ASA of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qwidgetplugin.h"

#ifndef QT_NO_WIDGETPLUGIN
#include "qwidgetinterface_p.h"
#include "qobjectcleanuphandler.h"
#include "qwidget.h"
#ifdef QT_CONTAINER_CUSTOM_WIDGETS
#include "qwidgetlist.h"
#endif

/*!
    \class QWidgetPlugin qwidgetplugin.h
    \brief The QWidgetPlugin class provides an abstract base for custom QWidget plugins.

    \ingroup plugins

    The widget plugin is a simple plugin interface that makes it easy
    to create custom widgets that can be included in forms using \link
    designer-manual.book Qt Designer\endlink and used by applications.

    Writing a widget plugin is achieved by subclassing this base
    class, reimplementing the pure virtual functions keys(), create(),
    group(), iconSet(), includeFile(), toolTip(), whatsThis() and
    isContainer(), and exporting the class with the \c Q_EXPORT_PLUGIN
    macro.

    See the \link designer-manual.book Qt Designer manual's\endlink,
    'Creating Custom Widgets' section in the 'Creating Custom Widgets'
    chapter, for a complete example of a QWidgetPlugin.

    See also the \link plugins-howto.html Plugins
    documentation\endlink and the \l{QWidgetFactory} class that is
    supplied with \link designer-manual.book Qt Designer\endlink.
*/

class QWidgetPluginPrivate : public QWidgetFactoryInterface,
#ifdef QT_CONTAINER_CUSTOM_WIDGETS
			     public QWidgetContainerInterfacePrivate,
#endif
			     private QLibraryInterface
{
public:
    QWidgetPluginPrivate( QWidgetPlugin *p )
	: plugin( p )
    {
    }

    virtual ~QWidgetPluginPrivate();

    QRESULT queryInterface( const QUuid &iid, QUnknownInterface **iface );
    Q_REFCOUNT;

    QStringList featureList() const;
    QWidget *create( const QString &key, QWidget *parent, const char *name );
    QString group( const QString &widget ) const;
    QIconSet iconSet( const QString &widget ) const;
    QString includeFile( const QString &widget ) const;
    QString toolTip( const QString &widget ) const;
    QString whatsThis( const QString &widget ) const;
    bool isContainer( const QString &widget ) const;
#ifdef QT_CONTAINER_CUSTOM_WIDGETS
    QWidget* containerOfWidget( const QString &key, QWidget *widget ) const;
    bool isPassiveInteractor( const QString &key, QWidget *widget ) const;
    bool supportsPages( const QString &key ) const;
    QWidget *addPage( const QString &key, QWidget *container, const QString &name, int index ) const;
    void insertPage( const QString &key, QWidget *container,
		     const QString &name, int index, QWidget *page ) const;
    void Page( const QString &key, QWidget *container,
	       const QString &name, int index, QWidget *page ) const;
    void removePage( const QString &key, QWidget *container, int index ) const;
    void movePage( const QString &key, QWidget *container, int fromIndex, int toIndex ) const;
    int count( const QString &key, QWidget *container ) const;
    int currentIndex( const QString &key, QWidget *container ) const;
    QString pageLabel( const QString &key, QWidget *container, int index ) const;
    QWidget *page( const QString &key, QWidget *container, int index ) const;
    void renamePage( const QString &key, QWidget *container, int index, const QString &newName ) const;
    QWidgetList pages( const QString &key, QWidget *container ) const;
    QString createCode( const QString &key, const QString &container,
			const QString &page, const QString &pageName ) const;
#endif // QT_CONTAINER_CUSTOM_WIDGETS
    bool init();
    void cleanup();
    bool canUnload() const;

private:
    QWidgetPlugin *plugin;
    QObjectCleanupHandler widgets;
};

QRESULT QWidgetPluginPrivate::queryInterface( const QUuid &iid, QUnknownInterface **iface )
{
    *iface = 0;

    if ( iid == IID_QUnknown )
	*iface = (QWidgetFactoryInterface*)this;
    else if ( iid == IID_QFeatureList )
	*iface = (QFeatureListInterface*)this;
    else if ( iid == IID_QWidgetFactory )
	*iface = (QWidgetFactoryInterface*)this;
    else if ( iid == IID_QLibrary )
	*iface = (QLibraryInterface*)this;
#ifdef QT_CONTAINER_CUSTOM_WIDGETS
    else if ( iid == IID_QWidgetContainer )
	*iface = (QWidgetContainerInterfacePrivate*)this;
#endif
    else
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
}

/*!
    \fn QStringList QWidgetPlugin::keys() const

    Returns the list of widget keys this plugin supports.

    These keys must be the class names of the custom widgets that are
    implemented in the plugin.

    \sa create()
*/

/*!
    \fn QWidget *QWidgetPlugin::create( const QString &, QWidget *, const char * )

    Creates and returns a QWidget object for the widget key \a key.
    The widget key is the class name of the required widget. The \a
    name and \a parent arguments are passed to the custom widget's
    constructor.

    \sa keys()
*/

QWidgetPluginPrivate::~QWidgetPluginPrivate()
{
    delete plugin;
}

QStringList QWidgetPluginPrivate::featureList() const
{
    return plugin->keys();
}

QWidget *QWidgetPluginPrivate::create( const QString &key, QWidget *parent, const char *name )
{
    QWidget *w = plugin->create( key, parent, name );
    widgets.add( w );
    return w;
}

QString QWidgetPluginPrivate::group( const QString &widget ) const
{
    return plugin->group( widget );
}

QIconSet QWidgetPluginPrivate::iconSet( const QString &widget ) const
{
    return plugin->iconSet( widget );
}

QString QWidgetPluginPrivate::includeFile( const QString &widget ) const
{
    return plugin->includeFile( widget );
}

QString QWidgetPluginPrivate::toolTip( const QString &widget ) const
{
    return plugin->toolTip( widget );
}

QString QWidgetPluginPrivate::whatsThis( const QString &widget ) const
{
    return plugin->whatsThis( widget );
}

bool QWidgetPluginPrivate::isContainer( const QString &widget ) const
{
    return plugin->isContainer( widget );
}

bool QWidgetPluginPrivate::init()
{
    return TRUE;
}

void QWidgetPluginPrivate::cleanup()
{
    widgets.clear();
}

bool QWidgetPluginPrivate::canUnload() const
{
    return widgets.isEmpty();
}

#ifdef QT_CONTAINER_CUSTOM_WIDGETS
QWidget* QWidgetPluginPrivate::containerOfWidget( const QString &key, QWidget *widget ) const
{
    QWidgetContainerPlugin *p = (QWidgetContainerPlugin*)plugin->qt_cast( "QWidgetContainerPlugin" );
    if ( p )
	return p->containerOfWidget( key, widget );
    return widget;
}

int QWidgetPluginPrivate::count( const QString &key, QWidget *container ) const
{
    QWidgetContainerPlugin *p = (QWidgetContainerPlugin*)plugin->qt_cast( "QWidgetContainerPlugin" );
    if ( p )
	return p->count( key, container );
    return 0;
}

int QWidgetPluginPrivate::currentIndex( const QString &key, QWidget *container ) const
{
    QWidgetContainerPlugin *p = (QWidgetContainerPlugin*)plugin->qt_cast( "QWidgetContainerPlugin" );
    if ( p )
	return p->currentIndex( key, container );
    return -1;
}

QString QWidgetPluginPrivate::pageLabel( const QString &key, QWidget *container, int index ) const
{
    QWidgetContainerPlugin *p = (QWidgetContainerPlugin*)plugin->qt_cast( "QWidgetContainerPlugin" );
    if ( p )
	return p->pageLabel( key, container, index );
    return QString::null;
}

QWidget *QWidgetPluginPrivate::page( const QString &key, QWidget *container, int index ) const
{
    QWidgetContainerPlugin *p = (QWidgetContainerPlugin*)plugin->qt_cast( "QWidgetContainerPlugin" );
    if ( p )
	return p->page( key, container, index );
    return 0;
}

bool QWidgetPluginPrivate::isPassiveInteractor( const QString &key, QWidget *widget ) const
{
    QWidgetContainerPlugin *p = (QWidgetContainerPlugin*)plugin->qt_cast( "QWidgetContainerPlugin" );
    if ( p )
	return p->isPassiveInteractor( key, widget );
    return FALSE;
}

bool QWidgetPluginPrivate::supportsPages( const QString &key ) const
{
    QWidgetContainerPlugin *p = (QWidgetContainerPlugin*)plugin->qt_cast( "QWidgetContainerPlugin" );
    if ( p )
	return p->supportsPages( key );
    return 0;
}

QWidget *QWidgetPluginPrivate::addPage( const QString &key, QWidget *container,
				    const QString &name, int index ) const
{
    QWidgetContainerPlugin *p = (QWidgetContainerPlugin*)plugin->qt_cast( "QWidgetContainerPlugin" );
    if ( p )
	return p->addPage( key, container, name, index );
    return 0;
}

void QWidgetPluginPrivate::insertPage( const QString &key, QWidget *container,
				       const QString &name, int index, QWidget *page ) const
{
    QWidgetContainerPlugin *p = (QWidgetContainerPlugin*)plugin->qt_cast( "QWidgetContainerPlugin" );
    if ( p )
	p->insertPage( key, container, name, index, page );
}

void QWidgetPluginPrivate::removePage( const QString &key, QWidget *container, int index ) const
{
    QWidgetContainerPlugin *p = (QWidgetContainerPlugin*)plugin->qt_cast( "QWidgetContainerPlugin" );
    if ( p )
	p->removePage( key, container, index );
}

void QWidgetPluginPrivate::movePage( const QString &key, QWidget *container,
				     int fromIndex, int toIndex ) const
{
    QWidgetContainerPlugin *p = (QWidgetContainerPlugin*)plugin->qt_cast( "QWidgetContainerPlugin" );
    if ( p )
	p->movePage( key, container, fromIndex, toIndex );
}

void QWidgetPluginPrivate::renamePage( const QString &key, QWidget *container,
				      int index, const QString &newName ) const
{
    QWidgetContainerPlugin *p = (QWidgetContainerPlugin*)plugin->qt_cast( "QWidgetContainerPlugin" );
    if ( p )
	p->renamePage( key, container, index, newName );
}

QWidgetList QWidgetPluginPrivate::pages( const QString &key, QWidget *container ) const
{
    QWidgetContainerPlugin *p = (QWidgetContainerPlugin*)plugin->qt_cast( "QWidgetContainerPlugin" );
    if ( p )
	return p->pages( key, container );
    return QWidgetList();
}

QString QWidgetPluginPrivate::createCode( const QString &key, const QString &container,
					  const QString &page, const QString &pageName ) const
{
    QWidgetContainerPlugin *p = (QWidgetContainerPlugin*)plugin->qt_cast( "QWidgetContainerPlugin" );
    if ( p )
	return p->createCode( key, container, page, pageName );
    return QString::null;
}
#endif // QT_CONTAINER_CUSTOM_WIDGETS

/*!
    Constructs a widget plugin. This is invoked automatically by the
    \c Q_EXPORT_PLUGIN macro.
*/
QWidgetPlugin::QWidgetPlugin()
    : QGPlugin( (QWidgetFactoryInterface*)(d = new QWidgetPluginPrivate( this )) )
{
}

/*!
    Destroys the widget plugin.

    You never have to call this explicitly. Qt destroys a plugin
    automatically when it is no longer used.
*/
QWidgetPlugin::~QWidgetPlugin()
{
    // don't delete d, as this is deleted by d
}

/*!
    Returns the group (toolbar name) that the custom widget of class
    \a key should be part of when \e{Qt Designer} loads it.

    The default implementation returns QString::null.
*/
QString QWidgetPlugin::group( const QString & ) const
{
    return QString::null;
}

/*!
    Returns the iconset that \e{Qt Designer} should use to represent
    the custom widget of class \a key in the toolbar.

    The default implementation returns an null iconset.
*/
QIconSet QWidgetPlugin::iconSet( const QString & ) const
{
    return QIconSet();
}

/*!
    Returns the name of the include file that \e{Qt Designer} and \c
    uic should use to include the custom widget of class \a key in
    generated code.

    The default implementation returns QString::null.
*/
QString QWidgetPlugin::includeFile( const QString & ) const
{
    return QString::null;
}

/*!
    Returns the text of the tooltip that \e{Qt Designer} should use
    for the custom widget of class \a key's toolbar button.

    The default implementation returns QString::null.
*/
QString QWidgetPlugin::toolTip( const QString & ) const
{
    return QString::null;
}

/*!
    Returns the text of the whatsThis text that \e{Qt Designer} should
    use when the user requests whatsThis help for the custom widget of
    class \a key.

    The default implementation returns QString::null.
*/
QString QWidgetPlugin::whatsThis( const QString & ) const
{
    return QString::null;
}

/*!
    Returns TRUE if the custom widget of class \a key can contain
    other widgets, e.g. like QFrame; otherwise returns FALSE.

    The default implementation returns FALSE.
*/
bool QWidgetPlugin::isContainer( const QString & ) const
{
    return FALSE;
}

#ifdef QT_CONTAINER_CUSTOM_WIDGETS

/*!
    \class QWidgetContainerPlugin qwidgetplugin.h
    \brief The QWidgetContainerPlugin class provides an abstract base
    for complex custom container QWidget plugins.

    \internal

    \ingroup plugins

    The widget container plugin is a subclass of QWidgetPlugin and
    extends the interface with functions necessary for supporting
    complex container widgets via plugins. These container widgets are
    widgets that have one or multiple sub widgets which act as the
    widget's containers. If the widget has multiple container
    subwidgets, they are referred to as "pages", and only one can be
    active at a time. Examples of complex container widgets include:
    QTabWidget, QWidgetStack and QToolBox.

    Writing a complex container widget plugin is achieved by
    subclassing this base class. First by reimplementing
    QWidgetPlugin's pure virtual functions keys(), create(), group(),
    iconSet(), includeFile(), toolTip(), whatsThis() and
    isContainer(), and exporting the class with the \c Q_EXPORT_PLUGIN
    macro. In addition containerOfWidget(), isPassiveInteractor() and
    supportsPages() must be reimplemented. If the widget
    supportsPages(), count(), currentIndex(), pageLabel(), page(),
    pages() and createCode() must be implemented. If the widget
    supportsPages() and you want to allow the containers pages to be
    modified, you must also reimplement addPage(), insertPage(),
    removePage(), movePage() and renamePage().

    \sa QWidgetPlugin
*/

/*!
    Constructs a complex container widget plugin. This is invoked
    automatically by the \c Q_EXPORT_PLUGIN macro.
*/

QWidgetContainerPlugin::QWidgetContainerPlugin()
    : QWidgetPlugin()
{
}

/*!
    Destroys the complex container widget plugin.

    You never have to call this explicitly. Qt destroys a plugin
    automatically when it is no longer used.
*/

QWidgetContainerPlugin::~QWidgetContainerPlugin()
{
}

/*!
    Operates on the plugin's \a key class.

    Returns the current \a container's custom widget. If the custom
    widget is a tab widget, this function takes the \a container as
    input and returns the widget's current page.

    The default implementation returns \a container.
*/

QWidget* QWidgetContainerPlugin::containerOfWidget( const QString &,
						    QWidget *container ) const
{
    return container;
}

/*!
    Operates on the plugin's \a key class.

    Returns the \a container custom widget's number of pages. If the
    custom widget is a tab widget, this function returns the number of
    tabs.

    The default implementation returns 0.
*/

int QWidgetContainerPlugin::count( const QString &, QWidget * ) const
{
    return 0;
}

/*!
    Operates on the plugin's \a key class.

    Returns the \a container custom widget's current page index. If
    the custom widget is a tab widget, this function returns the
    current tab's index.

    The default implementation returns -1.
*/

int QWidgetContainerPlugin::currentIndex( const QString &, QWidget * ) const
{
    return -1;
}

/*!
    Operates on the plugin's \a key class.

    Returns the \a container custom widget's label at position \a
    index. If the custom widget is a tab widget, this function returns
    the current tab's label.

    The default implementation returns a null string.
*/

QString QWidgetContainerPlugin::pageLabel( const QString &, QWidget *, int ) const
{
    return QString::null;
}

/*!
    Operates on the plugin's \a key class.

    Returns the \a container custom widget's page at position \a
    index. If the custom widget is a tab widget, this function returns
    the tab at index position \e index.


    The default implementation returns 0.
*/

QWidget *QWidgetContainerPlugin::page( const QString &, QWidget *, int ) const
{
    return 0;
}

/*!
    Operates on the plugin's \a key class.

    Returns TRUE if the \a container custom widget is a passive
    interactor for class \e key; otherwise returns FALSE. The \a
    container is a child widget of the actual custom widget.

    Usually, when a custom widget is used in \e{Qt Designer}'s design
    mode, no widget receives any mouse or key events, since \e{Qt
    Designer} filters and processes them itself. If one or more
    widgets of a custom widget still need to receive such events, for
    example, because the widget needs to change pages, this function
    must return TRUE for the widget. In such cases \e{Qt Designer}
    will not filter out key and mouse events destined for the widget.

    If the custom widget is a tab widget, the tab bar is the passive
    interactor, since that's what the user will use to change pages.

    The default implementation returns FALSE.
*/

bool QWidgetContainerPlugin::isPassiveInteractor( const QString &,
						 QWidget *container ) const
{
    Q_UNUSED( container )
    return FALSE;
}

/*!
    Operates on the plugin's \a key class.

    Returns TRUE if the widget supports pages; otherwise returns
    FALSE. If the custom widget is a tab widget this function should
    return TRUE.

    The default implementation returns FALSE.
*/

bool QWidgetContainerPlugin::supportsPages( const QString & ) const
{
    return FALSE;
}

/*!
    Operates on the plugin's \a key class.

    This function is called when a new page with the given \a name
    should be added to the \a container custom widget at position \a
    index.

    The default implementation does nothing.
*/

QWidget* QWidgetContainerPlugin::addPage( const QString &, QWidget *,
					  const QString &, int ) const
{
    return 0;
}

/*!
    Operates on the plugin's \a key class.

    This function is called when a new page, \a page, with the given
    \a name should be added to the \a container custom widget at
    position \a index.

    The default implementation does nothing.
*/

void QWidgetContainerPlugin::insertPage( const QString &, QWidget *,
					 const QString &, int, QWidget * ) const
{
}

/*!
    Operates on the plugin's \a key class.

    This function is called when the page at position \a index should
    be removed from the \a container custom widget.

    The default implementation does nothing.
*/

void QWidgetContainerPlugin::removePage( const QString &, QWidget *, int ) const
{
}

/*!
    Operates on the plugin's \a key class.

    This function is called when the page at position \a fromIndex should
    be moved to position \a toIndex in the \a container custom widget.

    The default implementation does nothing.
*/

void QWidgetContainerPlugin::movePage( const QString &, QWidget *, int, int ) const
{
}

/*!
    Operates on the plugin's \a key class.

    This function is called when the page at position \a index should
    be renamed (have its label changed) to \a newName in the \a
    container custom widget.

    The default implementation does nothing.
*/

void QWidgetContainerPlugin::renamePage( const QString &, QWidget *,
					 int, const QString & ) const
{
}

/*!
    Operates on the plugin's \a key class.

    This function should return a list of the \a container custom
    widget's pages.
*/

QWidgetList QWidgetContainerPlugin::pages( const QString &, QWidget * ) const
{
    return QWidgetList();
}

/*!
    Operates on the plugin's \a key class.

    This function is called from \e{Qt Designer}'s User Interface
    Compiler \c uic, when generating C++ code for inserting a page in
    the \a container custom widget. The name of the page widget which
    should be inserted at the end of the container is \a page, and the
    label of the page should be \a pageName.

    If the custom widget was a QTabWidget, the implementation of this
    function should return:

    \code
    return widget + "->addTab( " + page + ", \"" + pageName + "\" )";
    \endcode

    Warning: If the code returned by this function contains invalid
    C++ syntax, the generated \c uic code will not compile.
*/

QString QWidgetContainerPlugin::createCode( const QString &, const QString &,
					    const QString &, const QString & ) const
{
    return QString::null;
}

#endif // QT_CONTAINER_CUSTOM_WIDGETS

#endif //QT_NO_WIDGETPLUGIN
