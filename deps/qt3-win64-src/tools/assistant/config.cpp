/**********************************************************************
** Copyright (C) 2000-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the Qt Assistant.
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
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "config.h"
#include "profile.h"
#include "docuparser.h"

#include <qapplication.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qsettings.h>
#include <qxml.h>

static Config *static_configuration = 0;

inline QString getVersionString()
{
    return QString::number( (QT_VERSION >> 16) & 0xff )
	+ "." + QString::number( (QT_VERSION >> 8) & 0xff );
}

Config::Config()
    : hideSidebar( FALSE ), profil( 0 ), maximized(FALSE)
{
    fontSiz = qApp->font().pointSize();
    if( !static_configuration ) {
	static_configuration = this;
    } else {
	qWarning( "Multiple configurations not allowed!" );
    }
}

Config *Config::loadConfig(const QString &profileFileName)
{
    Config *config = new Config();

    if (profileFileName.isEmpty()) { // no profile
	config->profil = Profile::createDefaultProfile();
	config->load();
	config->loadDefaultProfile();
	return config;
    }

    QFile file(profileFileName);
    if (!file.exists()) {
	qWarning( "File does not exist: " + profileFileName );
	return 0;
    }
    DocuParser *parser = DocuParser::createParser( profileFileName );
    if (!parser) {
	qWarning( "Failed to create parser for file: " + profileFileName );
	return 0;
    }
    if (parser->parserVersion() < DocuParser::Qt320) {
	qWarning( "File does not contain profile information" );
	return 0;
    }
    DocuParser320 *profileParser = static_cast<DocuParser320*>(parser);
    parser->parse(&file);
    config->profil = profileParser->profile();
    if (!config->profil) {
	qWarning( "Config::loadConfig(), no profile in: " + profileFileName );
	return 0;
    }
    config->profil->setProfileType(Profile::UserProfile);
    config->profil->setDocuParser(profileParser);
    config->load();
    return config;
}

Config *Config::configuration()
{
    Q_ASSERT( static_configuration );
    return static_configuration;
}

void Config::load()
{
    const QString key = "/Qt Assistant/" + getVersionString() + "/";
    const QString profkey = key + "Profile/" + profil->props["name"] + "/";

    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, "/Trolltech" );

    webBrows = settings.readEntry( key + "Webbrowser" );
    home = settings.readEntry( profkey + "Homepage" );
    pdfApp = settings.readEntry( key + "PDFApplication" );
    linkUnder = settings.readBoolEntry( key + "LinkUnderline", TRUE );
    linkCol = settings.readEntry( key + "LinkColor", "#0000FF" );
    src = settings.readListEntry( profkey + "Source" );
    sideBar = settings.readNumEntry( key + "SideBarPage" );
    if (qApp->type() != QApplication::Tty) {
	fontFam = settings.readEntry( key + "Family", qApp->font().family() );

	fontFix = settings.readEntry( key + "FixedFamily", "courier" );
	fontSiz = settings.readNumEntry( key + "Size", -1 );
	if ( fontSiz < 4 ) {
	    fontSiz = qApp->font().pointSize();
	}

	geom.setRect( settings.readNumEntry( key + "GeometryX", QApplication::desktop()->availableGeometry().x() ),
		      settings.readNumEntry( key + "GeometryY", QApplication::desktop()->availableGeometry().y() ),
		      settings.readNumEntry( key + "GeometryWidth", 800 ),
		      settings.readNumEntry( key + "GeometryHeight", 600 ) );
	maximized = settings.readBoolEntry( key + "GeometryMaximized", FALSE );
    }
    mainWinLayout = settings.readEntry( key + "MainwindowLayout" );
    rebuildDocs = settings.readBoolEntry( key + "RebuildDocDB", TRUE );

    profileNames = settings.entryList( key + "Profile" );
}

void Config::save()
{
    saveSettings();
    saveProfile( profil );
}

void Config::saveSettings()
{
    const QString key = "/Qt Assistant/" + getVersionString() + "/";
    const QString profkey = key + "Profile/" + profil->props["name"] + "/";

    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, "/Trolltech" );

    settings.writeEntry( key + "Webbrowser", webBrows );
    settings.writeEntry( profkey + "Homepage", home );
    settings.writeEntry( key + "PDFApplication", pdfApp );
    settings.writeEntry( key + "LinkUnderline", linkUnder );
    settings.writeEntry( key + "LinkColor", linkCol );
    settings.writeEntry( profkey + "Source", src );
    settings.writeEntry( key + "SideBarPage", sideBarPage() );
    if (qApp->type() != QApplication::Tty) {
	settings.writeEntry( key + "GeometryX", geom.x() );
	settings.writeEntry( key + "GeometryY", geom.y() );
	settings.writeEntry( key + "GeometryWidth", geom.width() );
	settings.writeEntry( key + "GeometryHeight", geom.height() );
	settings.writeEntry( key + "GeometryMaximized", maximized );
	settings.writeEntry( key + "Family",  fontFam );
	settings.writeEntry( key + "Size",  fontSiz < 4 ? qApp->font().pointSize() : fontSiz );
	settings.writeEntry( key + "FixedFamily", fontFix );
    }
    if ( !hideSidebar )
	settings.writeEntry( key + "MainwindowLayout", mainWinLayout );
    settings.writeEntry( key + "RebuildDocDB", rebuildDocs );
}

#ifdef ASSISTANT_DEBUG
static void dumpmap( const QMap<QString,QString> &m, const QString &header )
{
    qDebug( header );
    QMap<QString,QString>::ConstIterator it = m.begin();
    while (it != m.end()) {
	qDebug( "  " + it.key() + ":\t\t" + *it );
	++it;
    }
}
#endif

void Config::loadDefaultProfile()
{
    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, "/Trolltech" );
    const QString key = "/Qt Assistant/" + QString(QT_VERSION_STR) + "/Profile";
    const QString profKey = key + "/default/";

    if( settings.entryList( key + "/default" ).count() == 0 ) {
	return;
    }

    // Override the defaults with settings in registry.
    profil->icons.clear();
    profil->indexPages.clear();
    profil->imageDirs.clear();
    profil->docs.clear();
    profil->dcfTitles.clear();

    QStringList titles = settings.readListEntry( profKey + "Titles" );
    QStringList iconLst = settings.readListEntry( profKey + "DocIcons" );
    QStringList indexLst = settings.readListEntry( profKey + "IndexPages" );
    QStringList imgDirLst = settings.readListEntry( profKey + "ImageDirs" );
    QStringList dcfs = settings.readListEntry( profKey + "DocFiles" );

    QStringList::ConstIterator it = titles.begin();
    QValueListConstIterator<QString> iconIt = iconLst.begin();
    QValueListConstIterator<QString> indexIt = indexLst.begin();
    QValueListConstIterator<QString> imageIt = imgDirLst.begin();
    QValueListConstIterator<QString> dcfIt = dcfs.begin();
    for( ; it != titles.end();
	++it, ++iconIt, ++indexIt, ++imageIt, ++dcfIt )
    {
	profil->addDCFIcon( *it, *iconIt );
	profil->addDCFIndexPage( *it, *indexIt );
	profil->addDCFImageDir( *it, *imageIt );
	profil->addDCFTitle( *dcfIt, *it );
    }
#if ASSISTANT_DEBUG
    dumpmap( profil->icons, "Icons" );
    dumpmap( profil->indexPages, "IndexPages" );
    dumpmap( profil->imageDirs, "ImageDirs" );
    dumpmap( profil->dcfTitles, "dcfTitles" );
    qDebug( "Docfiles: \n  " + profil->docs.join( "\n  " ) );
#endif
}

void Config::saveProfile( Profile *profile )
{
    if (profil && profil->profileType() == Profile::UserProfile)
	return;
    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, "/Trolltech" );
    QString versionString = (profile->props["name"] == "default")
	? QString(QT_VERSION_STR)
	: getVersionString();
    const QString key = "/Qt Assistant/" + versionString + "/";
    const QString profKey = key + "Profile/" + profile->props["name"] + "/";

    QStringList indexes, icons, imgDirs, dcfs;
    QValueList<QString> titles = profile->dcfTitles.keys();
    QValueListConstIterator<QString> it = titles.begin();
    for ( ; it != titles.end(); ++it ) {
	indexes << profile->indexPages[*it];
	icons << profile->icons[*it];
	imgDirs << profile->imageDirs[*it];
	dcfs << profile->dcfTitles[*it];
    }

    settings.writeEntry( profKey + "Titles", titles );
    settings.writeEntry( profKey + "DocFiles", dcfs );
    settings.writeEntry( profKey + "IndexPages", indexes );
    settings.writeEntry( profKey + "DocIcons", icons );
    settings.writeEntry( profKey + "ImageDirs", imgDirs );

#if ASSISTANT_DEBUG
    qDebug( "Titles:\n  - " + ( (QStringList*) &titles )->join( "\n  - " ) );
    qDebug( "Docfiles:\n  - " + dcfs.join( "\n  - " ) );
    qDebug( "IndexPages:\n  - " + indexes.join( "\n  - " ) );
    qDebug( "DocIcons:\n  - " + icons.join( "\n  - " ) );
    qDebug( "ImageDirs:\n  - " + imgDirs.join( "\n  - " ) );
#endif
}

QStringList Config::mimePaths()
{
    static QStringList lst;

    if( lst.count() > 0 )
	return lst;

    for (QMap<QString,QString>::ConstIterator it = profil->dcfTitles.begin();
	 it != profil->dcfTitles.end(); ++it ) {

	// Mime source for .dcf file path
	QFileInfo info( *it );
	QString dcfPath = info.dirPath(TRUE);
	if (lst.contains(dcfPath) == 0)
	    lst << dcfPath;

	// Image dir for .dcf
	QString imgDir = QDir::convertSeparators( dcfPath + QDir::separator()
						  + profil->imageDirs[it.key()] );
	if (lst.contains(imgDir) == 0)
	    lst << imgDir;
    }
    return lst;
}

QStringList Config::profiles() const
{
    return profileNames;
}

QString Config::title() const
{
    return profil->props[ "title" ];
}

QString Config::aboutApplicationMenuText() const
{
    return profil->props[ "aboutmenutext" ];
}

QString Config::aboutURL() const
{
    return profil->props[ "abouturl" ];
}

QString Config::homePage() const
{
    return home.isEmpty() ? profil->props["startpage"] : home;
}

QStringList Config::source() const
{
    return src.size() == 0 ? QStringList(profil->props["startpage"]) : src;
}

QStringList Config::docFiles() const
{
    return profil->docs;
}

QPixmap Config::docIcon( const QString &title ) const
{
    // ### To allow qdoc generated dcf files to reference the doc icons from qmake_image_col
    if (!QFile::exists(profil->icons[title]))
	return QPixmap::fromMimeSource( QFileInfo(profil->icons[title]).fileName() );
    return QPixmap::fromMimeSource( profil->icons[title] );
}

QPixmap Config::applicationIcon() const
{
    return QPixmap::fromMimeSource( profil->props["applicationicon"] );
}

QStringList Config::docTitles() const
{
    return QStringList(profil->indexPages.keys());
}

QString Config::docImageDir( const QString &docfile ) const
{
    return profil->imageDirs[docfile];
}

QString Config::indexPage( const QString &title ) const
{
    return profil->indexPages
	[title];
}

void Config::hideSideBar( bool b )
{
    hideSidebar = b;
}

bool Config::sideBarHidden() const
{
    return hideSidebar;
}

QString Config::assistantDocPath() const
{
    return profil->props["assistantdocs"].isEmpty()
	? QString( qInstallPathDocs() ) + "/html"
	: profil->props["assistantdocs"];
}
