#ifndef __UIDIRECT3DTEXTURECANVS_H__
#define __UIDIRECT3DTEXTURECANVS_H__

#include "UIDirectDrawCanvas.h"

#include <ddraw.h>

class UIDirect3DPrimaryCanvas;

//----------------------------------------------------------------------

class UIDirect3DTextureCanvas :
public UIDirectDrawCanvas
{
public:

	static const char *              TypeName;

	                                 UIDirect3DTextureCanvas( const UISize &, const bool InSystemMemory = false );
	virtual                         ~UIDirect3DTextureCanvas();
	
	virtual bool                     IsA                          (const UITypeID) const;
	virtual UIBaseObject *           Clone                        () const;

	// SetRenderDevice must be called before Generate is called so that we
	// use a valid pixel format.
	virtual void                     SetRenderCanvas              (UIDirect3DPrimaryCanvas *);
	virtual UIDirect3DPrimaryCanvas *GetRenderCanvas              () const;

	virtual void                     NotifyRenderCanvasChanged    ();
	
	virtual bool                     Generate                     () const;
	virtual bool                     CreateFromSurfaceDescription (LPDDSURFACEDESC2);

	void                             SetHasShader                 (bool b);
	bool                             GetHasShader                 () const;

private:


	UIDirect3DTextureCanvas & operator=                 (const UIDirect3DTextureCanvas &);
	                          UIDirect3DTextureCanvas   (const UIDirect3DTextureCanvas &);

	UIDirect3DPrimaryCanvas *mRenderCanvas;
	bool                     mInSystemMemory;
	bool                     mHasShader;
};

//----------------------------------------------------------------------

inline bool UIDirect3DTextureCanvas::GetHasShader                 () const
{
	return mHasShader;
}

//----------------------------------------------------------------------

#endif // __UIDIRECT3DTEXTURECANVS_H__