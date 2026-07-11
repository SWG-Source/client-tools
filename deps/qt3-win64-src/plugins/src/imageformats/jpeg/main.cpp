#ifndef QT_CLEAN_NAMESPACE
#define QT_CLEAN_NAMESPACE
#endif
#include <qimageformatplugin.h>

#ifndef QT_NO_IMAGEFORMATPLUGIN

#ifdef QT_NO_IMAGEIO_JPEG
#undef QT_NO_IMAGEIO_JPEG
#endif
#include "../../../../src/kernel/qjpegio.cpp"

class JPEGFormat : public QImageFormatPlugin
{
public:
    JPEGFormat();

    QStringList keys() const;
    bool loadImage( const QString &format, const QString &filename, QImage * );
    bool saveImage( const QString &format, const QString &filename, const QImage & );
    bool installIOHandler( const QString & );
};

JPEGFormat::JPEGFormat()
{
}


QStringList JPEGFormat::keys() const
{
    QStringList list;
    list << "JPEG";

    return list;
}

bool JPEGFormat::loadImage( const QString &format, const QString &filename, QImage *image )
{
    if ( format != "JPEG" )
	return FALSE;

    QImageIO io;
    io.setFileName( filename );
    io.setImage( *image );

    read_jpeg_image( &io );

    return TRUE;
}

bool JPEGFormat::saveImage( const QString &format, const QString &filename, const QImage &image )
{
    if ( format != "JPEG" )
	return FALSE;

    QImageIO io;
    io.setFileName( filename );
    io.setImage( image );

    write_jpeg_image( &io );

    return TRUE;
}

bool JPEGFormat::installIOHandler( const QString &name )
{
    if ( name.upper() != "JPEG" )
	return FALSE;

    qInitJpegIO();
    return TRUE;
}

Q_EXPORT_PLUGIN( JPEGFormat )

#endif // QT_NO_IMAGEFORMATPLUGIN
