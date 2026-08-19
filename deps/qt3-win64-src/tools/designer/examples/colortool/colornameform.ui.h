/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions use Qt Designer which will
** update this file, preserving your code. Create an init() function in place
** of a constructor, and a destroy() function in place of a destructor.
*****************************************************************************/
#include <qcolor.h>
#include <qmap.h>
#include <qstring.h>

QMap<QString,QColor> m_colors;

void ColorNameForm::setColors( const QMap<QString,QColor>& colors )
{
    m_colors = colors;
}

void ColorNameForm::validate()
{
    QString name = colorLineEdit->text();
    if ( ! name.isEmpty() &&
	 ( m_colors.isEmpty() || ! m_colors.contains( name ) ) )
	accept();
    else
	colorLineEdit->selectAll();
}
