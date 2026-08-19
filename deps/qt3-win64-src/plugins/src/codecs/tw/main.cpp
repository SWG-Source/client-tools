#include <qtextcodecplugin.h>
#include <qtextcodec.h>
#include <qptrlist.h>

#include <qbig5codec.h>
#include <private/qfontcodecs_p.h>


class TWTextCodecs : public QTextCodecPlugin
{
public:
    TWTextCodecs() {}
    
    QStringList names() const { return QStringList() << "Big5" << "big5*-0"; }
    QValueList<int> mibEnums() const { return QValueList<int>() << 2026 << -2026; }
    QTextCodec *createForMib( int );
    QTextCodec *createForName( const QString & );
};

QTextCodec *TWTextCodecs::createForMib( int mib )
{
    switch (mib) {
    case -2026:
	return new QFontBig5Codec;
    case 2026:
	return new QBig5Codec;
    default:
	;
    }

    return 0;
}


QTextCodec *TWTextCodecs::createForName( const QString &name )
{
    if (name == "Big5")
	return new QBig5Codec;
    if (name == "big5*-0")
	return new QFontBig5Codec;

    return 0;
}


Q_EXPORT_PLUGIN( TWTextCodecs );

