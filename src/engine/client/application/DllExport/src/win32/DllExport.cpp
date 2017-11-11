// ======================================================================
//
// DllExport.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

// make windows.h more strict in the types of handles
#ifndef STRICT
#define STRICT 1
#endif

// trim down the amount of stuff windows.h includes
#define NOGDICAPMASKS
#define NOVIRTUALKEYCODE
#define NOKEYSTATES
#define NORASTEROPS
#define NOATOM
#define NOCOLOR
#define NODRAWTEXT
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOOPENFILE
#define NOSERVICE
#define NOSOUND
#define NOCOMM
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <wtypes.h>

// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"

#include "clientGraphics/DynamicIndexBuffer.h"
#include "clientGraphics/DynamicVertexBuffer.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/Material.h"
#include "clientGraphics/ShaderImplementation.h"
#include "clientGraphics/StaticIndexBuffer.h"
#include "clientGraphics/StaticShader.h"
#include "clientGraphics/StaticVertexBuffer.h"
#include "clientGraphics/Texture.h"
#include "clientGraphics/TextureFormatInfo.h"
#include "clientGraphics/VertexBufferVector.h"
#include "sharedDebug/DataLint.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/DebugKey.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedDebug/Profiler.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ConfigSharedFoundation.h"
#include "sharedFoundation/CrashReportInformation.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Fatal.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedMath/Transform.h"
#include "sharedMath/VectorArgb.h"
#include "sharedObject/Object.h"
#include "sharedSynchronization/Mutex.h"

// ======================================================================

void Fatal(const char *, ...)
{
	__asm int 3;
}

void DebugFatal(const char *, ...)
{
	__asm int 3;
}

void Warning(const char *, ...)
{
	__asm int 3;
}

// ======================================================================

void Report::setFlags(int)
{
	__asm int 3;
}

void Report::vprintf(const char *, va_list)
{
	__asm int 3;
}

void Report::printf(const char *, ...)
{
	__asm int 3;
}

// ======================================================================

bool ExitChain::isFataling()
{
	__asm int 3;
	return false;
}

// ======================================================================

bool ConfigSharedFoundation::getVerboseHardwareLogging()
{
	__asm int 3;
	return false;
}

// ======================================================================

Mutex::Mutex()
{
	__asm int 3;
}

Mutex::~Mutex()
{
	__asm int 3;
}

// ======================================================================

const TextureFormatInfo &TextureFormatInfo::getInfo(TextureFormat)
{
	__asm int 3;
	static TextureFormatInfo dummy;
	return dummy;
}

void TextureFormatInfo::setSupported(TextureFormat, bool)
{
	__asm int 3;
}

// ======================================================================

void *MemoryManager::allocate(size_t, uint32, bool, bool)
{
	__asm int 3;
	return NULL;
}

void  MemoryManager::free(void *, bool)
{
	__asm int 3;
}

void  MemoryManager::own(void *)
{
	__asm int 3;
}

// ======================================================================

bool DataLint::isEnabled()
{
	__asm int 3;
	return false;
}

// ======================================================================

void DebugFlags::registerFlag(bool &, const char *, const char *)
{
	__asm int 3;
}

void DebugFlags::registerFlag(bool &, const char *, const char *, ReportRoutine1, int)
{
	__asm int 3;
}

void DebugFlags::unregisterFlag(bool &)
{
	__asm int 3;
}

// ======================================================================

void DebugKey::registerFlag(bool &, const char *)
{
	__asm int 3;
}

// ----------------------------------------------------------------------

bool DebugKey::isPressed(int)
{
	__asm int 3;
	return false;
}

// ----------------------------------------------------------------------

bool DebugKey::isDown(int)
{
	__asm int 3;
	return false;
}

// ======================================================================

Material::Material()
{
	__asm int 3;
}

Material::~Material()
{
	__asm int 3;
}

// ======================================================================

MemoryBlockManager::MemoryBlockManager(const char *, bool, int, int, int, int)
{
	__asm int 3;
}

MemoryBlockManager::~MemoryBlockManager()
{
	__asm int 3;
}

int MemoryBlockManager::getElementSize() const
{
	__asm int 3;
	return 0;
}

void *MemoryBlockManager::allocate(bool)
{
	__asm int 3;
	return 0;
}

void MemoryBlockManager::free(void *)
{
	__asm int 3;
}

// ======================================================================

bool Os::isMainThread(void)
{
	__asm int 3;
	return false;
}

Os::ThreadId Os::getThreadId()
{
	__asm int 3;
	return 0;
}

// ======================================================================

Transform const Transform::identity;

void Transform::multiply(const Transform &, const Transform &)
{
	__asm int 3;
}

// ======================================================================

const char *Shader::getName() const
{
	__asm int 3;
	return NULL;
}

bool StaticShader::getMaterial(Tag, Material &) const
{
	__asm int 3;
	return false;
}

bool StaticShader::getTextureData(Tag, StaticShaderTemplate::TextureData &) const
{
	__asm int 3;
	return false;
}

bool StaticShader::getTexture(Tag, const Texture *&) const
{
	__asm int 3;
	return false;
}

bool StaticShader::getTextureCoordinateSet(Tag, uint8 &) const
{
	__asm int 3;
	return false;
}

bool StaticShader::getTextureFactor(Tag, uint32 &) const
{
	__asm int 3;
	return false;
}

bool StaticShader::getTextureScroll(Tag, StaticShaderTemplate::TextureScroll &) const
{
	__asm int 3;
	return false;
}

bool StaticShader::getAlphaTestReferenceValue(Tag, uint8 &) const
{
	__asm int 3;
	return false;
}

bool StaticShader::getStencilReferenceValue(Tag, uint32 &) const
{
	__asm int 3;
	return false;
}

bool StaticShader::containsPrecalculatedVertexLighting() const
{
	__asm int 3;
	return false;
}

// ======================================================================

void Texture::fetch() const
{
	__asm int 3;
}

void Texture::release() const
{
	__asm int 3;
}

// ======================================================================

DynamicIndexBufferGraphicsData::~DynamicIndexBufferGraphicsData()
{
	__asm int 3;
}

DynamicVertexBufferGraphicsData::~DynamicVertexBufferGraphicsData()
{
	__asm int 3;
}

HardwareIndexBuffer::~HardwareIndexBuffer()
{
	__asm int 3;
}

StaticIndexBuffer::StaticIndexBuffer(int)
: HardwareIndexBuffer(T_static)
{
	__asm int 3;
}

StaticIndexBuffer::~StaticIndexBuffer()
{
	__asm int 3;
}

StaticIndexBufferGraphicsData::~StaticIndexBufferGraphicsData()
{
	__asm int 3;
}

StaticShaderGraphicsData::~StaticShaderGraphicsData()
{
	__asm int 3;
}

ShaderImplementationGraphicsData::~ShaderImplementationGraphicsData()
{
	__asm int 3;
}

StaticVertexBufferGraphicsData::~StaticVertexBufferGraphicsData()
{
	__asm int 3;
}

TextureGraphicsData::~TextureGraphicsData()
{
	__asm int 3;
}

VertexBufferVectorGraphicsData::~VertexBufferVectorGraphicsData()
{
	__asm int 3;
}

ShaderImplementationPassVertexShaderGraphicsData::~ShaderImplementationPassVertexShaderGraphicsData()
{
	__asm int 3;
}

ShaderImplementationPassPixelShaderProgramGraphicsData::~ShaderImplementationPassPixelShaderProgramGraphicsData()
{
	__asm int 3;
}

char const * ShaderImplementationPassPixelShaderProgram::getFileName() const
{
	__asm int 3;
	return 0;
}

int ShaderImplementationPassPixelShaderProgram::getVersionMajor() const
{
	__asm int 3;
	return 0;
}

int ShaderImplementationPassPixelShaderProgram::getVersionMinor() const
{
	__asm int 3;
	return 0;
}

// ======================================================================

int ConfigFile::getKeyInt(const char *, const char *, int, bool)
{
	__asm int 3;
	return 0;
}

bool  ConfigFile::getKeyBool  (const char *, const char *, bool, bool)
{
	__asm int 3;
	return false;
}

// ======================================================================

real Clock::frameTime()
{
	__asm int 3;
	return 0.0f;
}

// ======================================================================

void Profiler::enter(char const *)
{
	__asm int 3;
}

void Profiler::leave(char const *)
{
	__asm int 3;
}

void Profiler::transfer(char const *, char const *)
{
	__asm int 3;
}

// ======================================================================

AbstractFile *TreeFile::open(const char *, AbstractFile::PriorityType, bool)
{
	__asm int 3;
	return NULL;
}

// ======================================================================

CrcString::CrcString()
{
	__asm int 3;
}

CrcString::~CrcString()
{
	__asm int 3;
}

bool CrcString::operator < (CrcString const &) const
{
	return false;
}

// ======================================================================

PersistentCrcString::PersistentCrcString(CrcString const &)
{
	__asm int 3;
}

PersistentCrcString::~PersistentCrcString()
{
	__asm int 3;
}


char const * PersistentCrcString::getString() const
{
	__asm int 3;
	return NULL;
}

void PersistentCrcString::clear()
{
	__asm int 3;
}

void PersistentCrcString::set(char const *, bool)
{
	__asm int 3;
}

void PersistentCrcString::set(char const *, uint32)
{
	__asm int 3;
}

// ======================================================================

TemporaryCrcString::TemporaryCrcString(char const *, bool)
{
	__asm int 3;
}

TemporaryCrcString::~TemporaryCrcString()
{
	__asm int 3;
}

char const * TemporaryCrcString::getString() const
{
	__asm int 3;
	return NULL;
}

void TemporaryCrcString::clear()
{
	__asm int 3;
}

void TemporaryCrcString::set(char const *, bool)
{
	__asm int 3;
}

void TemporaryCrcString::set(char const *, uint32)
{
	__asm int 3;
}

// ======================================================================

void Graphics::setLastError(char const *, char const *)
{
	__asm int 3;
}

// ======================================================================

bool Graphics::writeImage(char const *, int const, int const, int const, int const *, bool const, Gl_imageFormat const, Rectangle2d const *)
{
	__asm int 3;
	return true;
}

// ======================================================================

void CrashReportInformation::addStaticText(char const *, ...)
{
	__asm int 3;
}


// ======================================================================

PerformanceTimer::PerformanceTimer()
{
	__asm int 3;
}

PerformanceTimer::~PerformanceTimer()
{
	__asm int 3;
}

void PerformanceTimer::start()
{
	__asm int 3;
}

void PerformanceTimer::resume()
{
	__asm int 3;
}

void PerformanceTimer::stop()
{
	__asm int 3;
}

float PerformanceTimer::getElapsedTime() const
{
	__asm int 3;
	return 0.0f;
}

// ======================================================================

Transform const & Object::getTransform_o2w() const
{
	__asm int 3;
	return Transform::identity;
}

// ======================================================================

BOOL APIENTRY DllMain(HANDLE, DWORD, LPVOID)
{
	return TRUE;
}
