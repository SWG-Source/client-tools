//======================================================================
//
// CuiObjectTextManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiObjectTextManager_H
#define INCLUDED_CuiObjectTextManager_H

#include "UITypes.h"

class Camera;
class ClientObject;
class Object;
class UIImageStyle;
struct UIColor;
struct UIPoint;
class BoxExtent;
class Transform;

class CellProperty;
class TangibleObject;
class Vector;
class NetworkId;
class CachedNetworkId;

//======================================================================

class CuiObjectTextManager
{
public:
	static void  install ();
	static void  remove  ();
	static void  reset   ();

	static void  update  ();

	static Vector getCurrentObjectHeadPoint_o (const Object & obj);
	static void   getObjectFullName           (Unicode::String & name, const ClientObject & object);
	static void   getObjectFullName           (Unicode::String & name, const ClientObject & object, float rangeToCameraSquared);
	static void   drawObjectLabels            (const Camera & camera);
	static void   renderTextAbove             (const ClientObject & object, const Camera & camera, const Unicode::String & str, float opacity, const UIColor &textColor, UIImageStyle *imageStyle, UIImageStyle *imageStyle2);

	static bool   setNameRender               (const ClientObject & object, bool value, float timein, float timeout, bool overrideInfiniteTimeout);
	static bool   toggleNameRender            (const ClientObject & object, float timeout, bool overrideInfiniteTimeout);

	static bool   getObjectHeadPoint          (const Object & object, const Camera & camera, float offset, UIPoint & pt);

	static int    getObjectHeadPointOffset    (const NetworkId & id, int * lastFrameOffset);
	static void   setObjectHeadPointOffset    (const NetworkId & id, int offset);

	static int    getObjectHeadPointMaxY      (const NetworkId & id, int * lastFrameMaxY);
	static void   setObjectHeadPointMaxY      (const NetworkId & id, int offset);

	static bool   canSee                      (const Vector & sourcePos, const CellProperty * sourceCell, const Object & targetObject, float & timeFactor);
	static bool   canSee                      (const Object & targetObject, float & timeFactor);

	static Vector computeCurHeadPoint_o       (const Object & obj);

	static void   computeBestFitBoxExtent     (const Object & foundObject, BoxExtent & box);
	static void   computeTransformedBoxExtent (const Object & obj, BoxExtent & boxExtent);
	static void   transformBox                (BoxExtent & box, const Transform & transform);

	static UIImageStyle * getObjFactionIcon   (const TangibleObject & obj);

	static bool canSee(Object const * const obj);
	static void getVisibleObjects(stdvector<Object*>::fwd & objectVector);

	static void updateVisibleObjectList(float const frameTime);

	static void updateNameWidgetHeight(CachedNetworkId const & id, UIScalar const offset);
	static UIScalar getNameWidgetHeight(CachedNetworkId const & id);
};

//======================================================================

#endif
