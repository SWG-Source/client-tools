#include <qtextcodecplugin.h>
#include <qtextcodec.h>
#include <qptrlist.h>

#include <qeuckrcodec.h>
#include <private/qfontcodecs_p.h>


class KRTextCodecs : public QTextCodecPlugin
{
public:
    KRTextCodecs() {}

    QStringList names() const { return QStringList() << "eucKR" << "ksc5601.1987-0"; }
    QValueList<int> mibEnums() const { return QValueList<int>() << 38 << 36; }
    QTextCodec *createForMib( int );
    QTextCodec *createForName( const QString & );
};


QTextCodec *KRTextCodecs::createForMib( int mib )
{
    switch (mib) {
    case 36:
	return new QFontKsc5601Codec;
    case 38:
	return new QEucKrCodec;
    default:
	;
    }

    return 0;
}


QTextCodec *KRTextCodecs::createForName( const QString &name )
{
    if (name == "eucKR")
	return new QEucKrCodec;
    if (name == "ksc5601.1987-0")
	return new QFontKsc5601Codec;

    return 0;
}


Q_EXPORT_PLUGIN( KRTextCodecs );
