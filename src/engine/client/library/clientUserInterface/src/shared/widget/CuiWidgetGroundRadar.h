// ======================================================================
//
// CuiWidgetGroundRadar.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CuiWidgetGroundRadar_H
#define INCLUDED_CuiWidgetGroundRadar_H

#include "UIWidget.h"
#include "UIEventCallback.h"
#include "UINotification.h"
#include "sharedMath/Vector.h"

class ClientObject;
class ClientProceduralTerrainAppearance;
class Light;
class Object;
class ObjectList;
class Shader;
class Texture;
class UIImage;
class UITextStyle;
class VectorArgb;
struct UIColor;

// ======================================================================

/**
* CuiWidgetGroundRadar is responsible for displaying a overhead radar map as
* a shader, with blips rendered to indicate various objects.
*
* The radar display is masked to a circular region.
*/
struct CuiWidgetGroundRadarObjectRenderer;

class CuiWidgetGroundRadar : 
public UIWidget
{
public:

	static const char * const TypeName;

	struct PropertyName
	{
		static const UILowerString TextStyle;
	};
	
	void                      setClipToCircle        (bool clip);
	void                      setDrawBackground      (bool drawBackground);

	                          CuiWidgetGroundRadar   (); 
	                         ~CuiWidgetGroundRadar   ();
	virtual UIBaseObject *    Clone                  () const { return new CuiWidgetGroundRadar; }
	virtual	UIStyle *         GetStyle               () const;
	
	virtual void              Render                 (UICanvas &) const;
	

	bool                      SetProperty            (const UILowerString & Name, const UIString &Value );
	bool                      GetProperty            (const UILowerString & Name, UIString &Value ) const;

	void                      setRadarOverlayImages  (UIImage * image, UIImage *compass);
	
	void                      GetPropertyNames       (UIPropertyNameVector &In, bool forCopy ) const;

	void                      setAngle               (float angle, bool force);
	float                     getAngle               () const;
	float                     getPixelsToWorldRatio  () const;

	bool                      updateRadarShader      (const Vector & center, float range, const ClientProceduralTerrainAppearance * cmtat, bool force);

	virtual const char        *GetTypeName           () const { return TypeName; }
	void                      setObjectRenderer      (const CuiWidgetGroundRadarObjectRenderer * objectRenderer);

    const Vector &            getRadarWorldCenter    () const;
	float                     getRadarRangeSquared   () const;
	float                     getRadarRange          () const;

	void                      SetEnabled             ( bool const NewEnabled );

private:
	                          CuiWidgetGroundRadar   (const CuiWidgetGroundRadar & rhs);
							  CuiWidgetGroundRadar & operator= (const CuiWidgetGroundRadar & rhs);

private:

	UITextStyle *             m_textStyle;

	float                     m_angle;
	Unicode::String           m_compassText[4];

	static const int CP_N;
	static const int CP_W;
	static const int CP_S;
	static const int CP_E;
	
	mutable UISize            m_lastSize;

	bool                      m_clipToCircle;
	bool                      m_drawBackground;
	
	UIPoint                   m_compassPoints [4];
	UIFloatPoint              m_quadPoints [4];

	Texture *                 m_phonyShaderTexture;
	
	//-----------------------------------------------------------------
	
	struct RadarShaderInfo
	{
	public:
		
		RadarShaderInfo ();
		~RadarShaderInfo ();
		
		bool update (const Vector & center, float range, const ClientProceduralTerrainAppearance * cmtat, bool force, bool clip, const VectorArgb & clearColor, bool drawTerrain);
				
		RadarShaderInfo (const RadarShaderInfo &);
		RadarShaderInfo & operator= (const RadarShaderInfo &);
		
		Shader *    m_shader;
		int         m_shaderSize;
		
		float       m_range;
		
		Vector      m_origin;
		float       m_worldSize;
		Vector      m_center;

		UIFloatPoint   m_quadUVs [4];

		enum 
		{
			m_maxSize = 256
		};

	private:
		void      updateUVs ();
		Unicode::String m_rangeString;	
	};
	
	//-----------------------------------------------------------------
	RadarShaderInfo           m_radarShaderInfo;

	const CuiWidgetGroundRadarObjectRenderer * m_objectRenderer;

	bool                      m_shaderIsCleared;
};

//----------------------------------------------------------------------

struct CuiWidgetGroundRadarObjectRenderer
{
	CuiWidgetGroundRadarObjectRenderer ();
//	virtual void renderObjectOntoRadar (const ClientObject & obj, const UIPoint & center) const;
	virtual void renderObjects (float angle, float pixels_to_world_ratio, const UIPoint & screenCenter, const UIRect& widgetExtent);

	virtual ~CuiWidgetGroundRadarObjectRenderer () = 0 {}
private:
	CuiWidgetGroundRadarObjectRenderer & operator= (const CuiWidgetGroundRadarObjectRenderer & rhs);
	CuiWidgetGroundRadarObjectRenderer (const CuiWidgetGroundRadarObjectRenderer & rhs);
};

// ======================================================================

inline UIStyle * CuiWidgetGroundRadar::GetStyle( void ) const
{
	return 0;
}

//----------------------------------------------------------------------

inline void CuiWidgetGroundRadar::setObjectRenderer (const CuiWidgetGroundRadarObjectRenderer * objectRenderer)
{
	m_objectRenderer = objectRenderer;
}

//----------------------------------------------------------------------

inline float CuiWidgetGroundRadar::getAngle () const
{
	return m_angle;
} 

//----------------------------------------------------------------------

inline const Vector & CuiWidgetGroundRadar::getRadarWorldCenter () const
{
	return m_radarShaderInfo.m_center;
}

//----------------------------------------------------------------------

inline float CuiWidgetGroundRadar::getRadarRangeSquared () const
{
	return m_radarShaderInfo.m_range * m_radarShaderInfo.m_range;
}

//----------------------------------------------------------------------

inline float CuiWidgetGroundRadar::getRadarRange          () const
{
	return m_radarShaderInfo.m_range;
}

//-----------------------------------------------------------------

#endif

