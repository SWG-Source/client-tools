#include <qtextcodecplugin.h>
#include <qtextcodec.h>
#include <qptrlist.h>

#include <qeucjpcodec.h>
#include <qjiscodec.h>
#include <qsjiscodec.h>
#include <private/qfontcodecs_p.h>


class JPTextCodecs : public QTextCodecPlugin
{
public:
    JPTextCodecs() {}

    QStringList names() const { return QStringList() << "eucJP" << "JIS7" << "SJIS" << "jisx0208.1983-0"; }
    QValueList<int> mibEnums() const { return QValueList<int>() << 16 << 17 << 18 << 63; }
    QTextCodec *createForMib( int );
    QTextCodec *createForName( const QString & );
};

QTextCodec *JPTextCodecs::createForMib( int mib )
{
    switch (mib) {
    case 16:
	return new QJisCodec;
    case 17:
	return new QSjisCodec;
    case 18:
	return new QEucJpCodec;
    case 63:
	return new QFontJis0208Codec;
    default:
	;
    }

    return 0;
}


QTextCodec *JPTextCodecs::createForName( const QString &name )
{
    if (name == "JIS7")
	return new QJisCodec;
    if (name == "SJIS")
	return new QSjisCodec;
    if (name == "eucJP")
	return new QEucJpCodec;
    if (name == "jisx0208.1983-0")
	return new QFontJis0208Codec;

    return 0;
}


Q_EXPORT_PLUGIN( JPTextCodecs );
