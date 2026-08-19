#include "plugin.h"
#include "../widget/filechooser.h"

static const char *filechooser_pixmap[] = {
    "22 22 8 1",
    "  c Gray100",
    ". c Gray97",
    "X c #4f504f",
    "o c #00007f",
    "O c Gray0",
    "+ c none",
    "@ c Gray0",
    "# c Gray0",
    "++++++++++++++++++++++",
    "++++++++++++++++++++++",
    "++++++++++++++++++++++",
    "++++++++++++++++++++++",
    "+OOOOOOOOOOOOOOOOOOOO+",
    "OOXXXXXXXXXXXXXXXXXXOO",
    "OXX.          OO OO  O",
    "OX.      oo     O    O",
    "OX.      oo     O   .O",
    "OX  ooo  oooo   O    O",
    "OX    oo oo oo  O    O",
    "OX  oooo oo oo  O    O",
    "OX oo oo oo oo  O    O",
    "OX oo oo oo oo  O    O",
    "OX  oooo oooo   O    O",
    "OX            OO OO  O",
    "OO..................OO",
    "+OOOOOOOOOOOOOOOOOOOO+",
    "++++++++++++++++++++++",
    "++++++++++++++++++++++",
    "++++++++++++++++++++++",
    "++++++++++++++++++++++"
};

CustomWidgetPlugin::CustomWidgetPlugin()
{
}

QStringList CustomWidgetPlugin::keys() const
{
    QStringList list;
    list << "FileChooser";
    return list;
}

QWidget* CustomWidgetPlugin::create( const QString &key, QWidget* parent, const char* name )
{
    if ( key == "FileChooser" )
	return new FileChooser( parent, name );
    return 0;
}

QString CustomWidgetPlugin::group( const QString& feature ) const
{
    if ( feature == "FileChooser" )
	return "Input";
    return QString::null;
}

QIconSet CustomWidgetPlugin::iconSet( const QString& ) const
{
    return QIconSet( QPixmap( filechooser_pixmap ) );
}

QString CustomWidgetPlugin::includeFile( const QString& feature ) const
{
    if ( feature == "FileChooser" )
	return "filechooser.h";
    return QString::null;
}

QString CustomWidgetPlugin::toolTip( const QString& feature ) const
{
    if ( feature == "FileChooser" )
	return "File Chooser Widget";
    return QString::null;
}

QString CustomWidgetPlugin::whatsThis( const QString& feature ) const
{
    if ( feature == "FileChooser" )
	return "A widget to choose a file or directory";
    return QString::null;
}

bool CustomWidgetPlugin::isContainer( const QString& ) const
{
    return FALSE;
}


Q_EXPORT_PLUGIN( CustomWidgetPlugin )
