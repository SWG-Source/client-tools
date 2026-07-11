#include "sizeaware.h"
#include <qsettings.h>


SizeAware::SizeAware( QDialog *parent, const char *name, bool modal )
    : QDialog( parent, name, modal )
{
    if ( company().isEmpty() )
	setCompany( "UnknownCompany" );
    if ( settingsFile().isEmpty() ) 
	setSettingsFile( "UnknownFile" );

    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, "/" + company() );
    settings.insertSearchPath( QSettings::Unix, "/Opt/" + company() + "/share" );
    int width  = settings.readNumEntry( "/" + settingsFile() + "/width",  640 );
    int height = settings.readNumEntry( "/" + settingsFile() + "/height", 480 );
    resize( width, height );
}


SizeAware::~SizeAware()
{
    // NOOP
}

void SizeAware::destroy()
{
    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, "/" + company() );
    settings.insertSearchPath( QSettings::Unix, "/Opt/" + company() + "/share" );
    settings.writeEntry( "/" + settingsFile() + "/width",   width() );
    settings.writeEntry( "/" + settingsFile() + "/height",  height() );
    close( TRUE );
}



