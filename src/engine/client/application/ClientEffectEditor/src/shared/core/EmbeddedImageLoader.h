// ======================================================================
//
// EmbeddedImageLoader.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_EmbeddedImageLoader_H
#define INCLUDED_EmbeddedImageLoader_H

// ======================================================================

class QPixmap;

// ======================================================================

class EmbeddedImageLoader
{
public:

	static QPixmap  getPixmap(const std::string &imageName);

};

// ======================================================================

#endif
