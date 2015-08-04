#ifndef INCLUDED_Headless_H
#define INCLUDED_Headless_H

struct Gl_install;
class Texture;
enum TextureFormat;
class TextureGraphicsData;
class StaticShader;
class StaticShaderGraphicsData;
class DynamicVertexBufferGraphicsData;
class DynamicVertexBuffer;
class StaticIndexBufferGraphicsData;
class StaticIndexBuffer;
class StaticVertexBufferGraphicsData;
class StaticVertexBuffer;
class DynamicIndexBuffer;
class DynamicIndexBufferGraphicsData;
class VertexBufferVectorGraphicsData;
class VertexBufferVector;
class ShaderImplementationGraphicsData;
class ShaderImplementation;

class Headless
{
public:
	static void _defaultFunc_Void();
	static int  _defaultFunc_True();

	static bool install( Gl_install *gl_install );
	static bool verify();

	static int  getShaderCapability();
	static bool requiresVertexAndPixelShaders();
	static int  getVideoMemoryInMegabytes();
	static void getOtherAdapterRects( stdvector< RECT > :: fwd & );

	static TextureGraphicsData *            createTextureData(
		const Texture &texture,
		const TextureFormat *runtimeFormats,
		int numberOfRuntimeFormats );

	static StaticShaderGraphicsData *       createStaticShaderGraphicsData( const StaticShader &shader );
	static DynamicVertexBufferGraphicsData *createDynamicVertexBufferData(const DynamicVertexBuffer &vb);
	static StaticVertexBufferGraphicsData * createStaticVertexBufferData( const StaticVertexBuffer &vb );
	static StaticIndexBufferGraphicsData *  createStaticIndexBufferData( const StaticIndexBuffer &buf );
	static DynamicIndexBufferGraphicsData * createDynamicIndexBufferData();
	static VertexBufferVectorGraphicsData * createVertexBufferVectorData( const VertexBufferVector & vbv );
	static ShaderImplementationGraphicsData* createShaderImplementationData( const ShaderImplementation &si );




};
#endif // INCLUDED_Headless_H