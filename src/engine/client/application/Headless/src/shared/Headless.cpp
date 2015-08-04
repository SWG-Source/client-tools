#include "FirstHeadless.h"
#include "Headless.h"
#include "Headless_TextureGraphicsData.h"
#include "Headless_StaticShaderData.h"
#include "Headless_DynamicVertexBufferData.h"
#include "Headless_StaticIndexBufferData.h"
#include "Headless_StaticVertexBufferData.h"
#include "Headless_DynamicIndexBufferData.h"
#include "Headless_VertexBufferVectorData.h"
#include "VertexBufferDescriptorCache.h"

#define PLATFORM_WIN32

#include "sharedFoundation/Tag.h"
#include "clientGraphics/Texture.def"
#include "clientGraphics/Texture.h"
#include "clientGraphics/Gl_dll.def"
#include "clientGraphics/ShaderCapability.h"

#include "clientGraphics/ShaderImplementation.h"

//---------------------------------------------------------------------------

namespace
{
	Gl_api            s_api;
	const int         cs_shaderMajor      = 1;
	const int         cs_shaderMinor      = 4;
	const int         cs_videoMemory      = 1024; // (value in megabytes) 1GB of video memory enough for you?
}

extern "C" __declspec(dllexport) Gl_api const * GetApi();

// ----------------------------------------------------------------------


bool Headless::install( Gl_install *gl_install )
{
	UNREF( gl_install );

	// set all of the pointers in the Gl_api to point to _defaultFunc

	int numPointers = sizeof( Gl_api ) / sizeof( void * );

	for ( int i = 0; i < numPointers; ++i )
	{
		((void**)&s_api)[i] = _defaultFunc_Void;
	}

	// now install the methods that we're going to overload

	s_api.createTextureData                      = createTextureData;
	s_api.createStaticShaderGraphicsData         = createStaticShaderGraphicsData;
	s_api.createDynamicVertexBufferData          = createDynamicVertexBufferData;
	s_api.createStaticIndexBufferData            = createStaticIndexBufferData;
	s_api.createStaticVertexBufferData           = createStaticVertexBufferData;
	s_api.getShaderCapability                    = getShaderCapability;
	s_api.createDynamicIndexBufferData           = createDynamicIndexBufferData;
	s_api.createVertexBufferVectorData           = createVertexBufferVectorData;
	s_api.createShaderImplementationGraphicsData = createShaderImplementationData;

	// set these to something that will return true


	#define DEFAULT_FUNC_TRUE( a ) 	*(int(**)())(&s_api.##a) = _defaultFunc_True;

	DEFAULT_FUNC_TRUE( presentToWindow );
	DEFAULT_FUNC_TRUE( present );
	DEFAULT_FUNC_TRUE( copyRenderTargetToNonRenderTargetTexture );
	DEFAULT_FUNC_TRUE( screenShot );
	DEFAULT_FUNC_TRUE( setMouseCursor );
	DEFAULT_FUNC_TRUE( writeImage );


	VertexBufferDescriptorCache::install();
	
	return true;
}

//---------------------------------------------------------------------------

bool Headless::verify()
{
	return true;
}

//---------------------------------------------------------------------------

void Headless::_defaultFunc_Void()
{
	__asm xor eax,eax;
}

//---------------------------------------------------------------------------

int Headless::_defaultFunc_True()
{
	return 1;
}

//---------------------------------------------------------------------------

ShaderImplementationGraphicsData *Headless::createShaderImplementationData( const ShaderImplementation & /*si*/ )
{
	return new ShaderImplementationGraphicsData();
}

//---------------------------------------------------------------------------

TextureGraphicsData *Headless::createTextureData( const Texture &texture, const TextureFormat *runtimeFormats, int numberOfRuntimeFormats )
{
	UNREF(texture);
	UNREF(numberOfRuntimeFormats);
	
	return new Headless_TextureGraphicsData( runtimeFormats[0] );
}

//---------------------------------------------------------------------------

StaticShaderGraphicsData *Headless::createStaticShaderGraphicsData( const StaticShader &shader )
{
	UNREF( shader );
	return new Headless_StaticShaderData();
}

//---------------------------------------------------------------------------

DynamicVertexBufferGraphicsData *Headless::createDynamicVertexBufferData(const DynamicVertexBuffer &vertexBuffer )
{
	return new Headless_DynamicVertexBufferData( vertexBuffer );
}

//---------------------------------------------------------------------------

StaticVertexBufferGraphicsData *Headless::createStaticVertexBufferData( const StaticVertexBuffer &vb )
{
	return new Headless_StaticVertexBufferData( vb );
}

//---------------------------------------------------------------------------

StaticIndexBufferGraphicsData *Headless::createStaticIndexBufferData( const StaticIndexBuffer &buf )
{
	return new Headless_StaticIndexBufferData( buf );
}

//---------------------------------------------------------------------------

int Headless::getShaderCapability()
{
	return ShaderCapability( cs_shaderMajor, cs_shaderMinor );
}

//---------------------------------------------------------------------------

bool Headless::requiresVertexAndPixelShaders()
{
	return false;
}

//---------------------------------------------------------------------------

int Headless::getVideoMemoryInMegabytes()
{
	return cs_videoMemory;
}


//---------------------------------------------------------------------------

DynamicIndexBufferGraphicsData *Headless::createDynamicIndexBufferData()
{
	return new Headless_DynamicIndexBufferData();
}

//---------------------------------------------------------------------------

VertexBufferVectorGraphicsData *Headless::createVertexBufferVectorData( const VertexBufferVector & )
{
	return new Headless_VertexBufferVectorData();
}

//---------------------------------------------------------------------------

Gl_api const * GetApi()
{
	s_api.verify  = Headless::verify;
	s_api.install = Headless::install;
	return &s_api;
}

//===========================================================================