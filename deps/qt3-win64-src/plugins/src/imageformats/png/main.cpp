#ifndef QT_CLEAN_NAMESPACE
#define QT_CLEAN_NAMESPACE
#endif
#include <qimageformatplugin.h>

#ifndef QT_NO_IMAGEFORMATPLUGIN

#ifdef QT_NO_IMAGEIO_PNG
#undef QT_NO_IMAGEIO_PNG
#endif
#include "../../../../src/kernel/qpngio.cpp"

class PNGFormat : public QImageFormatPlugin
{
public:
    PNGFormat();

    QStringList keys() const;
    bool loadImage( const QString &format, const QString &filename, QImage * );
    bool saveImage( const QString &format, const QString &filename, const QImage& );
    bool installIOHandler( const QString & );
};

PNGFormat::PNGFormat()
{
}


QStringList PNGFormat::keys() const
{
    QStringList list;
    list << "PNG";

    return list;
}

bool PNGFormat::loadImage( const QString &format, const QString &filename, QImage *image )
{
    if ( format != "PNG" )
	return FALSE;

    QImageIO io;
    io.setFileName( filename );
    io.setImage( *image );

    read_png_image( &io );

    return TRUE;
}

bool PNGFormat::saveImage( const QString &format, const QString &filename, const QImage &image )
{
    if ( format != "PNG" )
	return FALSE;

    QImageIO io;
    io.setFileName( filename );
    io.setImage( image );

    write_png_image( &io );

    return TRUE;
}

bool PNGFormat::installIOHandler( const QString &name )
{
    if ( name != "PNG" ) 
	return FALSE;

    qInitPngIO();
    return TRUE;
}

Q_EXPORT_PLUGIN( PNGFormat )

#endif // QT_NO_IMAGEFORMATPLUGIN
