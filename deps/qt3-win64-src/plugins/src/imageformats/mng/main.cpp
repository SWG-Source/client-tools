#ifndef QT_CLEAN_NAMESPACE
#define QT_CLEAN_NAMESPACE
#endif

#include <qimageformatplugin.h>

#ifndef QT_NO_IMAGEFORMATPLUGIN

#ifdef QT_NO_IMAGEIO_MNG
#undef QT_NO_IMAGEIO_MNG
#endif
#include "../../../../src/kernel/qmngio.cpp"

class MNGFormat : public QImageFormatPlugin
{
public:
    MNGFormat();

    QStringList keys() const;
    bool loadImage( const QString &format, const QString &filename, QImage *image );
    bool saveImage( const QString &format, const QString &filename, const QImage &image );
    bool installIOHandler( const QString & );
};

MNGFormat::MNGFormat()
{
}


QStringList MNGFormat::keys() const
{
    QStringList list;
    list << "MNG";

    return list;
}

bool MNGFormat::loadImage( const QString &, const QString &, QImage * )
{
    return FALSE;
}

bool MNGFormat::saveImage( const QString &, const QString &, const QImage& )
{
    return FALSE;
}

bool MNGFormat::installIOHandler( const QString &name )
{
    if ( name != "MNG" )
	return FALSE;

    qInitMngIO();
    return TRUE;
}

Q_EXPORT_PLUGIN( MNGFormat )

#endif // QT_NO_IMAGEFORMATPLUGIN
