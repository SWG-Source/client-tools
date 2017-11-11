//===================================================================
//
// DynamicNormalMapTest.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_DynamicNormalMapTest_H
#define INCLUDED_DynamicNormalMapTest_H

//===================================================================

#include "clientGame/TestIoWin.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Timer.h"

class Shader;
class Texture;
class DynamicHeightMap;
class DynamicNormalMap;

//===================================================================

class DynamicNormalMapTest : public TestIoWin
{
public:

	DynamicNormalMapTest ();
	virtual ~DynamicNormalMapTest ();

	virtual IoResult processEvent (IoEvent* event);
	virtual void     draw () const;

private:

	Shader*           m_dynamicShader;
	Texture*          m_dynamicTexture;
	DynamicHeightMap* m_waterHeightMap;
	DynamicNormalMap* m_waterNormalMap;
	Timer             m_timer;

	const Texture*    m_gradientSkyTexture;
	Texture*          m_dynamicCubeTexture;
	Timer             m_dynamicCubeTimer;

private:

	virtual void    createFloor ();
	virtual Object* createObjectAndAppearance ();

	void            updateDynamicCubeTexture ();

private:

	DynamicNormalMapTest (const DynamicNormalMapTest&);
	DynamicNormalMapTest& operator= (const DynamicNormalMapTest&);
};

//===================================================================

#endif
