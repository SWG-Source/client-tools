// ======================================================================
//
// EmbeddedImageLoader.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "AnimationEditor/FirstAnimationEditor.h"
#include "AnimationEditor/EmbeddedImageLoader.h"

#include <qdragobject.h>
#include <qmime.h>
#include <qpixmap.h>

// ======================================================================

QPixmap EmbeddedImageLoader::getPixmap(const std::string &imageName)
{
	QPixmap  pixmap;

	//-- Load the pixmap from the embedded resources.
	if (imageName.empty())
		return pixmap;

	const QMimeSource *const mimeSource = QMimeSourceFactory::defaultFactory()->data(imageName.c_str());
	if (mimeSource)
		IGNORE_RETURN(QImageDrag::decode(mimeSource, pixmap));

	return pixmap;
};

// ======================================================================
