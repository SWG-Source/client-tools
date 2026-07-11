#include "hello.h"

MyPushButton::MyPushButton( const QString& text ) 
    : QPushButton( text, 0, "mypushbutton" )
{
    qDebug( "My PushButton has been constructed" );
}
