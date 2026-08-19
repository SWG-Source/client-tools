#include <qstyleplugin.h>
#include <qmotifplusstyle.h>

class MotifPlusStyle : public QStylePlugin
{
public:
    MotifPlusStyle();

    QStringList keys() const;
    QStyle *create( const QString& );

};

MotifPlusStyle::MotifPlusStyle()
: QStylePlugin()
{
}

QStringList MotifPlusStyle::keys() const
{
    QStringList list;
    list << "MotifPlus";
    return list;
}

QStyle* MotifPlusStyle::create( const QString& s )
{
    if ( s.lower() == "motifplus" )
	return new QMotifPlusStyle();

    return 0;
}

Q_EXPORT_PLUGIN( MotifPlusStyle )

