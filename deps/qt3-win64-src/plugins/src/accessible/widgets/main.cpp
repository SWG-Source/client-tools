#include "qaccessiblewidget.h"
#include "qaccessiblemenu.h"

#include <qtoolbutton.h>
#include <qtoolbar.h>
#include <qvariant.h>

class AccessibleFactory : public QAccessibleFactoryInterface, public QLibraryInterface
{
public:
    AccessibleFactory();

    QRESULT queryInterface( const QUuid &, QUnknownInterface **iface );
    Q_REFCOUNT

    QStringList featureList() const;
    QRESULT createAccessibleInterface( const QString &classname, QObject *object, QAccessibleInterface **iface );

    bool init();
    void cleanup();
    bool canUnload() const;
};

AccessibleFactory::AccessibleFactory()
{
}

QRESULT AccessibleFactory::queryInterface( const QUuid &iid, QUnknownInterface **iface )
{
    *iface = 0;
    if ( iid == IID_QUnknown )
	*iface = (QUnknownInterface*)(QFeatureListInterface*)(QAccessibleFactoryInterface*)this;
    else if ( iid == IID_QFeatureList )
	*iface = (QFeatureListInterface*)this;
    else if ( iid == IID_QAccessibleFactory )
	*iface = (QAccessibleFactoryInterface*)this;
    else if ( iid == IID_QLibrary )
	*iface = (QLibraryInterface*)this;
    else
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
}

QStringList AccessibleFactory::featureList() const
{
    QStringList list;
    list << "QLineEdit";
    list << "QComboBox";
    list << "QSpinBox";
    list << "QSpinWidget";
    list << "QDial";
    list << "QScrollBar";
    list << "QSlider";
    list << "QToolButton";
    list << "QCheckBox";
    list << "QRadioButton";
    list << "QButton";
    list << "QViewportWidget";
    list << "QClipperWidget";
    list << "QTextEdit";
#ifndef QT_NO_ICONVIEW
    list << "QIconView";
#endif
    list << "QListView";
    list << "QListBox";
    list << "QTable";
    list << "QDialog";
    list << "QMessageBox";
    list << "QMainWindow";
    list << "QLabel";
    list << "QGroupBox";
    list << "QStatusBar";
    list << "QProgressBar";
    list << "QToolBar";
    list << "QMenuBar";
    list << "QPopupMenu";
    list << "QHeader";
    list << "QTabBar";
    list << "QTitleBar";
    list << "QWorkspaceChild";
    list << "QSizeGrip";
    list << "QSplitterHandle";
    list << "QToolBarSeparator";
    list << "QDockWindowHandle";
    list << "QDockWindowResizeHandle";
    list << "QTipLabel";
    list << "QFrame";
    list << "QWidgetStack";
    list << "QWidget";
    list << "QScrollView";

    return list;
}

QRESULT AccessibleFactory::createAccessibleInterface( const QString &classname, QObject *object, QAccessibleInterface **iface )
{
    *iface = 0;

    if ( classname == "QLineEdit" ) {
	*iface = new QAccessibleText( object, EditableText );
    } else if ( classname == "QComboBox" ) {
	*iface = new QAccessibleComboBox( object );
    } else if ( classname == "QSpinBox" ) {
	*iface = new QAccessibleRangeControl( object, SpinBox );
    } else if ( classname == "QSpinWidget" ) {
	*iface = new QAccessibleSpinWidget( object );
    } else if ( classname == "QDial" ) {
	*iface = new QAccessibleRangeControl( object, Dial );
    } else if ( classname == "QScrollBar" ) {
	*iface = new QAccessibleScrollBar( object );
    } else if ( classname == "QSlider" ) {
	*iface = new QAccessibleSlider( object );
    } else if ( classname == "QToolButton" ) {
	QToolButton *tb = (QToolButton*)object;
	if ( !tb->popup() )
	    *iface = new QAccessibleButton( object, PushButton );
	else if ( !tb->popupDelay() )
	    *iface = new QAccessibleButton( object, ButtonDropDown );
	else
	    *iface = new QAccessibleButton( object, ButtonMenu );
    } else if ( classname == "QCheckBox" ) {
	*iface = new QAccessibleButton( object, CheckBox );
    } else if ( classname == "QRadioButton" ) {
	*iface = new QAccessibleButton( object, RadioButton );
    } else if ( classname == "QButton" ) {
	*iface = new QAccessibleButton( object, PushButton );
    } else if ( classname == "QViewportWidget" ) {
	*iface = new QAccessibleViewport( object, object->parent() );
    } else if ( classname == "QClipperWidget" ) {
	*iface = new QAccessibleViewport( object, object->parent()->parent() );
    } else if ( classname == "QTextEdit" ) {
	*iface = new QAccessibleTextEdit( object );
#ifndef QT_NO_ICONVIEW
    } else if ( classname == "QIconView" ) {
	*iface = new QAccessibleIconView( object );
#endif
    } else if ( classname == "QListView" ) {
	*iface = new QAccessibleListView( object );
    } else if ( classname == "QListBox" ) {
	*iface = new QAccessibleListBox( object );
    } else if ( classname == "QTable" ) {
	*iface = new QAccessibleScrollView( object, Table );
    } else if ( classname == "QDialog" ) {
	*iface = new QAccessibleWidget( object, Dialog );
    } else if ( classname == "QMessageBox" ) {
	*iface = new QAccessibleWidget( object, AlertMessage );
    } else if ( classname == "QMainWindow" ) {
	*iface = new QAccessibleWidget( object, Application );
    } else if ( classname == "QLabel" || classname == "QLCDNumber" ) {
	*iface = new QAccessibleDisplay( object, StaticText );
    } else if ( classname == "QGroupBox" ) {
	*iface = new QAccessibleDisplay( object, Grouping );
    } else if ( classname == "QStatusBar" ) {
	*iface = new QAccessibleWidget( object, StatusBar );
    } else if ( classname == "QProgressBar" ) {
	*iface = new QAccessibleRangeControl( object, ProgressBar );
    } else if ( classname == "QToolBar" ) {
	QToolBar *tb = (QToolBar*)object;
	*iface = new QAccessibleWidget( object, ToolBar, tb->label() );
    } else if ( classname == "QMenuBar" ) {
	*iface = new QAccessibleMenuBar( object );
    } else if ( classname == "QPopupMenu" ) {
	*iface = new QAccessiblePopup( object );
    } else if ( classname == "QHeader" ) {
	*iface = new QAccessibleHeader( object );
    } else if ( classname == "QTabBar" ) {
	*iface = new QAccessibleTabBar( object );
    } else if ( classname == "QTitleBar" ) {
	*iface = new QAccessibleTitleBar( object );
    } else if ( classname == "QWorkspaceChild" ) {
	*iface = new QAccessibleWidget( object, Window );
    } else if ( classname == "QSizeGrip" ) {
	*iface = new QAccessibleWidget( object, Grip );
    } else if ( classname == "QSplitterHandle" ) {
	*iface = new QAccessibleWidget( object, Separator, QString::null, 
					QString::null, QString::null, QString::null, 
					QString::null, QString::null, QAccessible::Moveable );
    } else if ( classname == "QToolBarSeparator" ) {
	*iface = new QAccessibleWidget( object, QAccessible::Separator, QString::null, 
					QString::null, QString::null, QString::null, 
					QString::null, QString::null, QAccessible::Unavailable );
    } else if ( classname == "QDockWindowHandle" ) {
	*iface = new QAccessibleWidget( object, QAccessible::Grip, object->property( "caption" ).toString(),
					QString::null, QString::null, QString::null,
					QString::null, QString::null, QAccessible::Moveable );
    } else if ( classname == "QDockWindowResizeHandle" ) {
	*iface = new QAccessibleWidget( object, QAccessible::Separator, QString::null,
					QString::null, QString::null, QString::null,
					QString::null, QString::null, QAccessible::Moveable );
    } else if ( classname == "QTipLabel" ) {
	*iface = new QAccessibleWidget( object, ToolTip );
    } else if ( classname == "QFrame" ) {
	*iface = new QAccessibleWidget( object, Border );
    } else if ( classname == "QWidgetStack" ) {
	*iface = new QAccessibleWidgetStack( object );
    } else if ( classname == "QWidget" ) {
	*iface = new QAccessibleWidget( object );
    } else if ( classname == "QScrollView" ) {
        *iface = new QAccessibleScrollView( object, Client );
    } else
	return QE_NOINTERFACE;

    (*iface)->addRef();
    return QS_OK;
}

bool AccessibleFactory::init()
{
    return TRUE;
}

void AccessibleFactory::cleanup()
{
}

bool AccessibleFactory::canUnload() const
{
    return (QAccessibleWidget::objects == 0);
}

Q_EXPORT_COMPONENT()
{
    Q_CREATE_INSTANCE( AccessibleFactory )
}
