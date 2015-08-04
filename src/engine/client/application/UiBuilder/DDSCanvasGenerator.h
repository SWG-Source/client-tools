#ifndef __DDSCANVASGENRATOR_H__
#define __DDSCANVASGENRATOR_H__

#include "UICanvasGenerator.h"

class DDSCanvasGenerator : public UICanvasGenerator
{
public:
	
												 DDSCanvasGenerator( const UINarrowString & );
	virtual								~DDSCanvasGenerator();

	virtual bool					 GetSize( UISize & ) const;
	virtual bool					 GenerateOnto( UICanvas & ) const;

private:


	DDSCanvasGenerator & operator=            (const DDSCanvasGenerator &);
	                     DDSCanvasGenerator   (const DDSCanvasGenerator &);

	virtual bool					 InitializeCanvasFromMemory( UICanvas &DestinationCanvas, unsigned char *pbase ) const;

	UINarrowString							 mFilename;
};

#endif // __DDSCANVASGENRATOR_H__