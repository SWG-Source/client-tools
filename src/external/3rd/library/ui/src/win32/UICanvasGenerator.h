#ifndef __UICANVSGENERATOR_H__
#define __UICANVSGENERATOR_H__

#include "UIBaseObject.h"

class UICanvas;

// =====================================

class UICanvasGenerator : public UIBaseObject
{
public:
											  UICanvasGenerator();
	virtual							 ~UICanvasGenerator();
										
	virtual bool					IsA( const UITypeID ) const;
	virtual UIBaseObject *Clone( void ) const { return 0; };
									
	virtual bool				  GetSize( UISize & ) const = 0;
	virtual bool				  GenerateOnto( UICanvas & ) const = 0;

private:

	UICanvasGenerator & operator= (const UICanvasGenerator &);
	          UICanvasGenerator   (const UICanvasGenerator &);

};

// =====================================

class UICanvasFactory
{
public:

	virtual UICanvas *CreateCanvas( const std::string & shaderName, const std::string & textureName) const = 0;	
};

// =====================================

template <class T> class UIStandardCanvasFactory : public UICanvasFactory 
{
public:
	
	virtual UICanvas *CreateCanvas( const std::string & shaderName, const std::string & textureName) const
	{
		T * const NewGenerator = new T( textureName );
		UISize CanvasSize;
		
		if( NewGenerator->GetSize( CanvasSize ) )
		{
			UIDirect3DTextureCanvas *NewCanvas = new UIDirect3DTextureCanvas( CanvasSize );
			
			NewCanvas->SetRenderCanvas ( gPrimaryDisplay );
			NewCanvas->SetGenerator    ( NewGenerator );	
			NewCanvas->SetName         ( shaderName + ":" + textureName );
			NewCanvas->SetHasShader    (!shaderName.empty ());

			return NewCanvas;
		}
		
		delete NewGenerator;
		return 0;
	};
};

#endif // __UICANVSGENERATOR_H__

