//
// CuiLayer.h - the implementation layer for the PlanetSide UI system
// asommers 1-10-2001
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef CuiLAYER_H
#define CuiLAYER_H

//-------------------------------------------------------------------

#include "UICanvas.h"
#include "UISoundCanvas.h"
#include "UICanvasGenerator.h"
#include "UILocalizedStringFactory.h"

//-------------------------------------------------------------------

class Shader;
class Sound;
class SoundTemplate;
class StringId;

//-------------------------------------------------------------------

/**
* CuiLayer is the lowest level interface between the engine and the PS UI.
*
* It provides implementations for rendering textured quads, and lines,
* creating textures by name, loading UI resources from the treefile,
* and producing localized strings on demand.
*
*/

class CuiLayer
{
public:

	static void  install ();
	static void  remove ();

	class TextureCanvas;
	class EngineCanvas;

	//-------------------------------------------------------------------
	//
	// SoundCanvas
	//
	class SoundCanvas : public UISoundCanvas
	{
	public:

		                        SoundCanvas              ();
		                       ~SoundCanvas              ();
		virtual void            Play                     (const char* filename);
		virtual void            PlaySoundGenericNegative () const;

	private:

		SoundCanvas &           operator=   (const SoundCanvas &);
		                        SoundCanvas (const SoundCanvas &);

		typedef stdmap<std::string, const SoundTemplate *>::fwd SoundTemplateVector;
		SoundTemplateVector *         m_soundTemplates;

	};

	//----------------------------------------------------------------------

	class Loader;

	//-----------------------------------------------------------------
	//
	// 
	
	class TextureCanvasFactory : public UICanvasFactory
	{
	public:

		virtual UICanvas *CreateCanvas( const std::string & shaderName, const std::string & textureName) const;	
	};

	//-----------------------------------------------------------------

	class StringFactory : public UILocalizedStringFactory
	{
	public:
		virtual bool             GetLocalizedString (const UINarrowString & name, UIString & dest) const;
		                         StringFactory ();

		static void                      generateStringList ();
		typedef stdvector<StringId>::fwd StringIdVector;
		static const StringIdVector &    getStringList      ();

	private:
		                         StringFactory (const StringFactory & rhs);
		StringFactory &          operator= (const StringFactory & rhs);
	};

	class CursorInterface;

};

//-----------------------------------------------------------------

#endif
