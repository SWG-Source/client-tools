#include <qstyleplugin.h>
#include <qcdestyle.h>

class CDEStyle : public QStylePlugin
{
public:
    CDEStyle();

    QStringList keys() const;
    QStyle *create( const QString& );
};

CDEStyle::CDEStyle()
: QStylePlugin()
{
}

QStringList CDEStyle::keys() const
{
    QStringList list;
    list << "CDE";
    return list;
}

QStyle* CDEStyle::create( const QString& s )
{
    if ( s.lower() == "cde" )
	return new QCDEStyle();

    return 0;
}

Q_EXPORT_PLUGIN( CDEStyle )
