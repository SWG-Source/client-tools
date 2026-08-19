/****************************************************************************
** $Id: qjpegio.cpp 2051 2007-02-21 10:04:20Z chehrlic $
**
** Implementation of JPEG QImage IOHandler
**
** Created : 990521
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech ASA of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QT_CLEAN_NAMESPACE
#define QT_CLEAN_NAMESPACE
#endif

#include "qimage.h"

#ifndef QT_NO_IMAGEIO_JPEG

#include "qiodevice.h"
#include "qjpegio.h"

#include <stdio.h>      // jpeglib needs this to be pre-included
#include <setjmp.h>


// including jpeglib.h seems to be a little messy
extern "C" {
#define XMD_H           // shut JPEGlib up
#if defined(Q_OS_UNIXWARE)
#  define HAVE_BOOLEAN  // libjpeg under Unixware seems to need this
#endif
#include <jpeglib.h>
#ifdef const
#  undef const          // remove crazy C hackery in jconfig.h
#endif
}


struct my_error_mgr : public jpeg_error_mgr {
    jmp_buf setjmp_buffer;
};

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

static
void my_error_exit (j_common_ptr cinfo)
{
    my_error_mgr* myerr = (my_error_mgr*) cinfo->err;
    char buffer[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message)(cinfo, buffer);
    qWarning(buffer);
    longjmp(myerr->setjmp_buffer, 1);
}

#if defined(Q_C_CALLBACKS)
}
#endif


static const int max_buf = 4096;

struct my_jpeg_source_mgr : public jpeg_source_mgr {
    // Nothing dynamic - cannot rely on destruction over longjump
    QImageIO* iio;
    JOCTET buffer[max_buf];

public:
    my_jpeg_source_mgr(QImageIO* iio);
};

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

static
void qt_init_source(j_decompress_ptr)
{
}

static
boolean qt_fill_input_buffer(j_decompress_ptr cinfo)
{
    int num_read;
    my_jpeg_source_mgr* src = (my_jpeg_source_mgr*)cinfo->src;
    QIODevice* dev = src->iio->ioDevice();
    src->next_input_byte = src->buffer;
    num_read = dev->readBlock((char*)src->buffer, max_buf);
    if ( num_read <= 0 ) {
	// Insert a fake EOI marker - as per jpeglib recommendation
	src->buffer[0] = (JOCTET) 0xFF;
	src->buffer[1] = (JOCTET) JPEG_EOI;
	src->bytes_in_buffer = 2;
    } else {
	src->bytes_in_buffer = num_read;
    }
#if defined(Q_OS_UNIXWARE)
    return B_TRUE;
#else
    return TRUE;
#endif
}

static
void qt_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
    my_jpeg_source_mgr* src = (my_jpeg_source_mgr*)cinfo->src;

    // `dumb' implementation from jpeglib

    /* Just a dumb implementation for now.  Could use fseek() except
     * it doesn't work on pipes.  Not clear that being smart is worth
     * any trouble anyway --- large skips are infrequent.
     */
    if (num_bytes > 0) {
	while (num_bytes > (long) src->bytes_in_buffer) {
	    num_bytes -= (long) src->bytes_in_buffer;
	    (void) qt_fill_input_buffer(cinfo);
	    /* note we assume that qt_fill_input_buffer will never return FALSE,
	    * so suspension need not be handled.
	    */
	}
	src->next_input_byte += (size_t) num_bytes;
	src->bytes_in_buffer -= (size_t) num_bytes;
    }
}

static
void qt_term_source(j_decompress_ptr)
{
}

#if defined(Q_C_CALLBACKS)
}
#endif


inline my_jpeg_source_mgr::my_jpeg_source_mgr(QImageIO* iioptr)
{
    jpeg_source_mgr::init_source = qt_init_source;
    jpeg_source_mgr::fill_input_buffer = qt_fill_input_buffer;
    jpeg_source_mgr::skip_input_data = qt_skip_input_data;
    jpeg_source_mgr::resync_to_restart = jpeg_resync_to_restart;
    jpeg_source_mgr::term_source = qt_term_source;
    iio = iioptr;
    bytes_in_buffer = 0;
    next_input_byte = buffer;
}


static
void scaleSize( int &reqW, int &reqH, int imgW, int imgH, QImage::ScaleMode mode )
{
    if ( mode == QImage::ScaleFree )
        return;
    int t1 = imgW * reqH;
    int t2 = reqW * imgH;
    if (( mode == QImage::ScaleMin && (t1 > t2) ) || ( mode == QImage::ScaleMax && (t1 < t2) ))
	reqH = t2 / imgW;
    else
        reqW = t1 / imgH;
}


static
void read_jpeg_image(QImageIO* iio)
{
    QImage image;

    struct jpeg_decompress_struct cinfo;

    struct my_jpeg_source_mgr *iod_src = new my_jpeg_source_mgr(iio);
    struct my_error_mgr jerr;

    jpeg_create_decompress(&cinfo);

    cinfo.src = iod_src;

    cinfo.err = jpeg_std_error(&jerr);
    jerr.error_exit = my_error_exit;

    if (!setjmp(jerr.setjmp_buffer)) {
#if defined(Q_OS_UNIXWARE)
	(void) jpeg_read_header(&cinfo, B_TRUE);
#else
	(void) jpeg_read_header(&cinfo, TRUE);
#endif

	(void) jpeg_start_decompress(&cinfo);

	QString params = iio->parameters();
	params.simplifyWhiteSpace();
	int sWidth = 0, sHeight = 0;
	char sModeStr[1024] = "";
	QImage::ScaleMode sMode;

	if ( params.contains( "GetHeaderInformation" ) ) {

	    // Create QImage's without allocating the data
	    if ( cinfo.output_components == 3 || cinfo.output_components == 4) {
		image = QImage( NULL, cinfo.output_width, cinfo.output_height, 32, NULL, 0, QImage::IgnoreEndian );
	    } else if ( cinfo.output_components == 1 ) {
		image = QImage( NULL, cinfo.output_width, cinfo.output_height, 8, NULL, 0, QImage::IgnoreEndian );
	    } else {
		// Unsupported format
	    }


	} else if ( params.contains( "Scale" ) ) {
	    sscanf( params.latin1(), "Scale( %i, %i, %1023s )",
		    &sWidth, &sHeight, sModeStr );

	    QString sModeQStr( sModeStr );
	    if ( sModeQStr == "ScaleFree" ) {
		sMode = QImage::ScaleFree;
	    } else if ( sModeQStr == "ScaleMin" ) {
		sMode = QImage::ScaleMin;
	    } else if ( sModeQStr == "ScaleMax" ) {
		sMode = QImage::ScaleMax;
	    } else {
		qDebug("read_jpeg_image: invalid scale mode \"%s\", see QImage::ScaleMode documentation", sModeStr);
		sMode = QImage::ScaleFree;
	    }

//	    qDebug( "Parameters ask to scale the image to %i x %i ScaleMode: %s", sWidth, sHeight, sModeStr );
	    scaleSize( sWidth, sHeight, cinfo.output_width, cinfo.output_height, sMode );
//	    qDebug( "Scaling the jpeg to %i x %i", sWidth, sHeight, sModeStr );

	    bool created = FALSE;
	    if ( cinfo.output_components == 3 || cinfo.output_components == 4) {
		created = image.create( sWidth, sHeight, 32 );
	    } else if ( cinfo.output_components == 1 ) {
		created = image.create( sWidth, sHeight, 8, 256 );
		for (int i=0; i<256; i++)
		    image.setColor(i, qRgb(i,i,i));
	    } else {
		// Unsupported format
	    }
	    if (!created)
		image = QImage();

	    if (!image.isNull()) {
		QImage tmpImage( cinfo.output_width, 1, 32 );
		uchar** inLines = tmpImage.jumpTable();
		uchar** outLines = image.jumpTable();
		while (cinfo.output_scanline < cinfo.output_height) {
		    int outputLine = sHeight * cinfo.output_scanline / cinfo.output_height;
		    (void) jpeg_read_scanlines(&cinfo, inLines, 1);
		    if ( cinfo.output_components == 3 ) {
			uchar *in = inLines[0];
			QRgb *out = (QRgb*)outLines[outputLine];
			for (uint i=0; i<cinfo.output_width; i++ ) {
// ### Only scaling down an image works, I don't think scaling up will work at the moment
// ### An idea I have to make this a smooth scale is to progressively add the pixel values up
// When scaling down, multiple values are being over drawn in to the output buffer.
// Instead, a weighting based on the distance the line or pixel is from the output pixel determines
// the weight of it when added to the output buffer. At present it is a non-smooth scale which is
// inefficently implemented, it still uncompresses all the jpeg, an optimization for progressive
// jpegs could be made if scaling by say 50% or some other special cases
			    out[sWidth * i / cinfo.output_width] = qRgb( in[0], in[1], in[2] );
			    in += 3;
			}
		    } else {
// ### Need to test the case where the jpeg is grayscale, need some black and white jpegs to test
// this code. (also only scales down and probably won't scale to a larger size)
			uchar *in = inLines[0];
			uchar *out = outLines[outputLine];
			for (uint i=0; i<cinfo.output_width; i++ ) {
			    out[sWidth * i / cinfo.output_width] = in[i];
			}
		    }
		}
		(void) jpeg_finish_decompress(&cinfo);
	    }

	} else {

	    bool created = FALSE;
	    if ( cinfo.output_components == 3 || cinfo.output_components == 4) {
		created = image.create( cinfo.output_width, cinfo.output_height, 32 );
	    } else if ( cinfo.output_components == 1 ) {
		created = image.create( cinfo.output_width, cinfo.output_height, 8, 256 );
		for (int i=0; i<256; i++)
		    image.setColor(i, qRgb(i,i,i));
	    } else {
		// Unsupported format
	    }
	    if (!created)
		image = QImage();

	    if (!image.isNull()) {
		uchar** lines = image.jumpTable();
		while (cinfo.output_scanline < cinfo.output_height)
		    (void) jpeg_read_scanlines(&cinfo,
		                               lines + cinfo.output_scanline,
		                               cinfo.output_height);
		(void) jpeg_finish_decompress(&cinfo);

		if ( cinfo.output_components == 3 ) {
		    // Expand 24->32 bpp.
		    for (uint j=0; j<cinfo.output_height; j++) {
			uchar *in = image.scanLine(j) + cinfo.output_width * 3;
			QRgb *out = (QRgb*)image.scanLine(j);

			for (uint i=cinfo.output_width; i--; ) {
			    in-=3;
			    out[i] = qRgb(in[0], in[1], in[2]);
			}
		    }
		}
	    }
        }

	if (!image.isNull()) {
	    if ( cinfo.density_unit == 1 ) {
	        image.setDotsPerMeterX( int(100. * cinfo.X_density / 2.54) );
	        image.setDotsPerMeterY( int(100. * cinfo.Y_density / 2.54) );
	    } else if ( cinfo.density_unit == 2 ) {
		image.setDotsPerMeterX( int(100. * cinfo.X_density) );
		image.setDotsPerMeterY( int(100. * cinfo.Y_density) );
	    }
	}

	iio->setImage(image);
	iio->setStatus(image.isNull());
    }

    jpeg_destroy_decompress(&cinfo);
    delete iod_src;
}


struct my_jpeg_destination_mgr : public jpeg_destination_mgr {
    // Nothing dynamic - cannot rely on destruction over longjump
    QImageIO* iio;
    JOCTET buffer[max_buf];

public:
    my_jpeg_destination_mgr(QImageIO*);
};


#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

static
void qt_init_destination(j_compress_ptr)
{
}

static
void qt_exit_on_error(j_compress_ptr cinfo, QIODevice* dev)
{
    if (dev->status() == IO_Ok) {
	return;
    } else {
	// cinfo->err->msg_code = JERR_FILE_WRITE;
	(*cinfo->err->error_exit)((j_common_ptr)cinfo);
    }
}

static
boolean qt_empty_output_buffer(j_compress_ptr cinfo)
{
    my_jpeg_destination_mgr* dest = (my_jpeg_destination_mgr*)cinfo->dest;
    QIODevice* dev = dest->iio->ioDevice();

    if ( dev->writeBlock( (char*)dest->buffer, max_buf ) != max_buf )
	qt_exit_on_error(cinfo, dev);

    dest->next_output_byte = dest->buffer;
    dest->free_in_buffer = max_buf;

#if defined(Q_OS_UNIXWARE)
    return B_TRUE;
#else
    return TRUE;
#endif
}

static
void qt_term_destination(j_compress_ptr cinfo)
{
    my_jpeg_destination_mgr* dest = (my_jpeg_destination_mgr*)cinfo->dest;
    QIODevice* dev = dest->iio->ioDevice();
    Q_LONG n = max_buf - dest->free_in_buffer;

    if ( dev->writeBlock( (char*)dest->buffer, n ) != n )
	qt_exit_on_error(cinfo, dev);

    dev->flush();

    qt_exit_on_error(cinfo, dev);
}

#if defined(Q_C_CALLBACKS)
}
#endif


inline
my_jpeg_destination_mgr::my_jpeg_destination_mgr(QImageIO* iioptr)
{
    jpeg_destination_mgr::init_destination = qt_init_destination;
    jpeg_destination_mgr::empty_output_buffer = qt_empty_output_buffer;
    jpeg_destination_mgr::term_destination = qt_term_destination;
    iio = iioptr;
    next_output_byte = buffer;
    free_in_buffer = max_buf;
}


static
void write_jpeg_image(QImageIO* iio)
{
    QImage image = iio->image();

    struct jpeg_compress_struct cinfo;
    JSAMPROW row_pointer[1];
    row_pointer[0] = 0;

    struct my_jpeg_destination_mgr *iod_dest = new my_jpeg_destination_mgr(iio);
    struct my_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);

    jerr.error_exit = my_error_exit;

    if (!setjmp(jerr.setjmp_buffer)) {
	jpeg_create_compress(&cinfo);

	cinfo.dest = iod_dest;

	cinfo.image_width = image.width();
	cinfo.image_height = image.height();

	QRgb* cmap=0;
	bool gray=FALSE;
	switch ( image.depth() ) {
	  case 1:
	  case 8:
	    cmap = image.colorTable();
	    gray = TRUE;
	    int i;
	    for (i=image.numColors(); gray && i--; ) {
		gray = gray & ( qRed(cmap[i]) == qGreen(cmap[i]) &&
				qRed(cmap[i]) == qBlue(cmap[i]) );
	    }
	    cinfo.input_components = gray ? 1 : 3;
	    cinfo.in_color_space = gray ? JCS_GRAYSCALE : JCS_RGB;
	    break;
	  case 32:
	    cinfo.input_components = 3;
	    cinfo.in_color_space = JCS_RGB;
	}

	jpeg_set_defaults(&cinfo);

	float diffInch = QABS(image.dotsPerMeterX()*2.54/100. - qRound(image.dotsPerMeterX()*2.54/100.))
	                 + QABS(image.dotsPerMeterY()*2.54/100. - qRound(image.dotsPerMeterY()*2.54/100.));
	float diffCm = (QABS(image.dotsPerMeterX()/100. - qRound(image.dotsPerMeterX()/100.))
	                + QABS(image.dotsPerMeterY()/100. - qRound(image.dotsPerMeterY()/100.)))*2.54;
	if (diffInch < diffCm) {
	    cinfo.density_unit = 1; // dots/inch
	    cinfo.X_density = qRound(image.dotsPerMeterX()*2.54/100.);
	    cinfo.Y_density = qRound(image.dotsPerMeterY()*2.54/100.);
	} else {
	    cinfo.density_unit = 2; // dots/cm
	    cinfo.X_density = (image.dotsPerMeterX()+50) / 100;
	    cinfo.Y_density = (image.dotsPerMeterY()+50) / 100;
	}

	int quality = iio->quality() >= 0 ? QMIN(iio->quality(),100) : 75;
#if defined(Q_OS_UNIXWARE)
	jpeg_set_quality(&cinfo, quality, B_TRUE /* limit to baseline-JPEG values */);
	jpeg_start_compress(&cinfo, B_TRUE);
#else
	jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);
	jpeg_start_compress(&cinfo, TRUE);
#endif

	row_pointer[0] = new uchar[cinfo.image_width*cinfo.input_components];
	int w = cinfo.image_width;
	while (cinfo.next_scanline < cinfo.image_height) {
	    uchar *row = row_pointer[0];
	    switch ( image.depth() ) {
	      case 1:
		if (gray) {
		    uchar* data = image.scanLine(cinfo.next_scanline);
		    if ( image.bitOrder() == QImage::LittleEndian ) {
			for (int i=0; i<w; i++) {
			    bool bit = !!(*(data + (i >> 3)) & (1 << (i & 7)));
			    row[i] = qRed(cmap[bit]);
			}
		    } else {
			for (int i=0; i<w; i++) {
			    bool bit = !!(*(data + (i >> 3)) & (1 << (7 -(i & 7))));
			    row[i] = qRed(cmap[bit]);
			}
		    }
		} else {
		    uchar* data = image.scanLine(cinfo.next_scanline);
		    if ( image.bitOrder() == QImage::LittleEndian ) {
			for (int i=0; i<w; i++) {
			    bool bit = !!(*(data + (i >> 3)) & (1 << (i & 7)));
			    *row++ = qRed(cmap[bit]);
			    *row++ = qGreen(cmap[bit]);
			    *row++ = qBlue(cmap[bit]);
			}
		    } else {
			for (int i=0; i<w; i++) {
			    bool bit = !!(*(data + (i >> 3)) & (1 << (7 -(i & 7))));
			    *row++ = qRed(cmap[bit]);
			    *row++ = qGreen(cmap[bit]);
			    *row++ = qBlue(cmap[bit]);
			}
		    }
		}
		break;
	      case 8:
		if (gray) {
		    uchar* pix = image.scanLine(cinfo.next_scanline);
		    for (int i=0; i<w; i++) {
			*row = qRed(cmap[*pix]);
			++row; ++pix;
		    }
		} else {
		    uchar* pix = image.scanLine(cinfo.next_scanline);
		    for (int i=0; i<w; i++) {
			*row++ = qRed(cmap[*pix]);
			*row++ = qGreen(cmap[*pix]);
			*row++ = qBlue(cmap[*pix]);
			++pix;
		    }
		}
		break;
	      case 32: {
		QRgb* rgb = (QRgb*)image.scanLine(cinfo.next_scanline);
		for (int i=0; i<w; i++) {
		    *row++ = qRed(*rgb);
		    *row++ = qGreen(*rgb);
		    *row++ = qBlue(*rgb);
		    ++rgb;
		}
	      }
	    }
	    jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);

	iio->setStatus(0);
    }

    delete iod_dest;
    delete [] row_pointer[0];
}

void qInitJpegIO()
{
    // Not much to go on - just 3 bytes: 0xFF, M_SOI, 0xFF
    // Even the third is not strictly specified as required.
    QImageIO::defineIOHandler("JPEG", "^\377\330\377", 0, read_jpeg_image, write_jpeg_image);
}

#endif
