#ifndef INCLUDED_VertexBufferDescriptorCache_H
#define INCLUDED_VertexBufferDescriptorCache_H

struct VertexBufferDescriptor;
class VertexBufferFormat;

class VertexBufferDescriptorCache
{
public:
	static void                          install();
	static void                          shutdown();
	static const VertexBufferDescriptor &getDescriptor( const VertexBufferFormat &format );
};

#endif // INCLUDED_VertexBufferDescriptorCache_H