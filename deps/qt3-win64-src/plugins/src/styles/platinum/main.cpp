#include <qplatinumstyle.h>
#include <qstyleplugin.h>

class PlatinumStyle : public QStylePlugin
{
public:
    PlatinumStyle();

    QStringList keys() const;
    QStyle *create( const QString& );
};

PlatinumStyle::PlatinumStyle()
: QStylePlugin()
{
}

QStringList PlatinumStyle::keys() const
{
    QStringList list;
    list << "Platinum";
    return list;
}

QStyle* PlatinumStyle::create( const QString& s )
{
    if ( s.lower() == "platinum" )
        return new QPlatinumStyle();

    return 0;
}


Q_EXPORT_PLUGIN( PlatinumStyle )
