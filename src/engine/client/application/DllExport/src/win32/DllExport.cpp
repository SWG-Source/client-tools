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
	__debugbreak();
}

void DebugFatal(const char *, ...)
{
	__debugbreak();
}

void Warning(const char *, ...)
{
	__debugbreak();
}

// ======================================================================

void Report::setFlags(int)
{
	__debugbreak();
}

void Report::vprintf(const char *, va_list)
{
	__debugbreak();
}

void Report::printf(const char *, ...)
{
	__debugbreak();
}

// ======================================================================

bool ExitChain::isFataling()
{
	__debugbreak();
	return false;
}

// ======================================================================

bool ConfigSharedFoundation::getVerboseHardwareLogging()
{
	__debugbreak();
	return false;
}

// ======================================================================

Mutex::Mutex()
{
	__debugbreak();
}

Mutex::~Mutex()
{
	__debugbreak();
}

// ======================================================================

const TextureFormatInfo &TextureFormatInfo::getInfo(TextureFormat)
{
	__debugbreak();
	static TextureFormatInfo dummy;
	return dummy;
}

void TextureFormatInfo::setSupported(TextureFormat, bool)
{
	__debugbreak();
}

// ======================================================================

void *MemoryManager::allocate(size_t, uint32, bool, bool)
{
	__debugbreak();
	return NULL;
}

void  MemoryManager::free(void *, bool)
{
	__debugbreak();
}

void  MemoryManager::own(void *)
{
	__debugbreak();
}

// ======================================================================

bool DataLint::isEnabled()
{
	__debugbreak();
	return false;
}

// ======================================================================

void DebugFlags::registerFlag(bool &, const char *, const char *)
{
	__debugbreak();
}

void DebugFlags::registerFlag(bool &, const char *, const char *, ReportRoutine1, int)
{
	__debugbreak();
}

void DebugFlags::unregisterFlag(bool &)
{
	__debugbreak();
}

// ======================================================================

void DebugKey::registerFlag(bool &, const char *)
{
	__debugbreak();
}

// ----------------------------------------------------------------------

bool DebugKey::isPressed(int)
{
	__debugbreak();
	return false;
}

// ----------------------------------------------------------------------

bool DebugKey::isDown(int)
{
	__debugbreak();
	return false;
}

// ======================================================================

Material::Material()
{
	__debugbreak();
}

Material::~Material()
{
	__debugbreak();
}

// ======================================================================

MemoryBlockManager::MemoryBlockManager(const char *, bool, int, int, int, int)
{
	__debugbreak();
}

MemoryBlockManager::~MemoryBlockManager()
{
	__debugbreak();
}

int MemoryBlockManager::getElementSize() const
{
	__debugbreak();
	return 0;
}

void *MemoryBlockManager::allocate(bool)
{
	__debugbreak();
	return 0;
}

void MemoryBlockManager::free(void *)
{
	__debugbreak();
}

// ======================================================================

bool Os::isMainThread(void)
{
	__debugbreak();
	return false;
}

Os::ThreadId Os::getThreadId()
{
	__debugbreak();
	return 0;
}

// ======================================================================

Transform const Transform::identity;

void Transform::multiply(const Transform &, const Transform &)
{
	__debugbreak();
}

// ======================================================================

const char *Shader::getName() const
{
	__debugbreak();
	return NULL;
}

bool StaticShader::getMaterial(Tag, Material &) const
{
	__debugbreak();
	return false;
}

bool StaticShader::getTextureData(Tag, StaticShaderTemplate::TextureData &) const
{
	__debugbreak();
	return false;
}

bool StaticShader::getTexture(Tag, const Texture *&) const
{
	__debugbreak();
	return false;
}

bool StaticShader::getTextureCoordinateSet(Tag, uint8 &) const
{
	__debugbreak();
	return false;
}

bool StaticShader::getTextureFactor(Tag, uint32 &) const
{
	__debugbreak();
	return false;
}

bool StaticShader::getTextureScroll(Tag, StaticShaderTemplate::TextureScroll &) const
{
	__debugbreak();
	return false;
}

bool StaticShader::getAlphaTestReferenceValue(Tag, uint8 &) const
{
	__debugbreak();
	return false;
}

bool StaticShader::getStencilReferenceValue(Tag, uint32 &) const
{
	__debugbreak();
	return false;
}

bool StaticShader::containsPrecalculatedVertexLighting() const
{
	__debugbreak();
	return false;
}

// ======================================================================

void Texture::fetch() const
{
	__debugbreak();
}

void Texture::release() const
{
	__debugbreak();
}

// ======================================================================

DynamicIndexBufferGraphicsData::~DynamicIndexBufferGraphicsData()
{
	__debugbreak();
}

DynamicVertexBufferGraphicsData::~DynamicVertexBufferGraphicsData()
{
	__debugbreak();
}

HardwareIndexBuffer::~HardwareIndexBuffer()
{
	__debugbreak();
}

StaticIndexBuffer::StaticIndexBuffer(int)
: HardwareIndexBuffer(T_static)
{
	__debugbreak();
}

StaticIndexBuffer::~StaticIndexBuffer()
{
	__debugbreak();
}

StaticIndexBufferGraphicsData::~StaticIndexBufferGraphicsData()
{
	__debugbreak();
}

StaticShaderGraphicsData::~StaticShaderGraphicsData()
{
	__debugbreak();
}

ShaderImplementationGraphicsData::~ShaderImplementationGraphicsData()
{
	__debugbreak();
}

StaticVertexBufferGraphicsData::~StaticVertexBufferGraphicsData()
{
	__debugbreak();
}

TextureGraphicsData::~TextureGraphicsData()
{
	__debugbreak();
}

VertexBufferVectorGraphicsData::~VertexBufferVectorGraphicsData()
{
	__debugbreak();
}

ShaderImplementationPassVertexShaderGraphicsData::~ShaderImplementationPassVertexShaderGraphicsData()
{
	__debugbreak();
}

ShaderImplementationPassPixelShaderProgramGraphicsData::~ShaderImplementationPassPixelShaderProgramGraphicsData()
{
	__debugbreak();
}

char const * ShaderImplementationPassPixelShaderProgram::getFileName() const
{
	__debugbreak();
	return 0;
}

int ShaderImplementationPassPixelShaderProgram::getVersionMajor() const
{
	__debugbreak();
	return 0;
}

int ShaderImplementationPassPixelShaderProgram::getVersionMinor() const
{
	__debugbreak();
	return 0;
}

// ======================================================================

int ConfigFile::getKeyInt(const char *, const char *, int, bool)
{
	__debugbreak();
	return 0;
}

bool  ConfigFile::getKeyBool  (const char *, const char *, bool, bool)
{
	__debugbreak();
	return false;
}

// ======================================================================

real Clock::frameTime()
{
	__debugbreak();
	return 0.0f;
}

// ======================================================================

void Profiler::enter(char const *)
{
	__debugbreak();
}

void Profiler::leave(char const *)
{
	__debugbreak();
}

void Profiler::transfer(char const *, char const *)
{
	__debugbreak();
}

// ======================================================================

AbstractFile *TreeFile::open(const char *, AbstractFile::PriorityType, bool)
{
	__debugbreak();
	return NULL;
}

// ======================================================================

CrcString::CrcString()
{
	__debugbreak();
}

CrcString::~CrcString()
{
	__debugbreak();
}

bool CrcString::operator < (CrcString const &) const
{
	return false;
}

// ======================================================================

PersistentCrcString::PersistentCrcString(CrcString const &)
{
	__debugbreak();
}

PersistentCrcString::~PersistentCrcString()
{
	__debugbreak();
}


char const * PersistentCrcString::getString() const
{
	__debugbreak();
	return NULL;
}

void PersistentCrcString::clear()
{
	__debugbreak();
}

void PersistentCrcString::set(char const *, bool)
{
	__debugbreak();
}

void PersistentCrcString::set(char const *, uint32)
{
	__debugbreak();
}

// ======================================================================

TemporaryCrcString::TemporaryCrcString(char const *, bool)
{
	__debugbreak();
}

TemporaryCrcString::~TemporaryCrcString()
{
	__debugbreak();
}

char const * TemporaryCrcString::getString() const
{
	__debugbreak();
	return NULL;
}

void TemporaryCrcString::clear()
{
	__debugbreak();
}

void TemporaryCrcString::set(char const *, bool)
{
	__debugbreak();
}

void TemporaryCrcString::set(char const *, uint32)
{
	__debugbreak();
}

// ======================================================================

void Graphics::setLastError(char const *, char const *)
{
	__debugbreak();
}

// ======================================================================

bool Graphics::writeImage(char const *, int const, int const, int const, int const *, bool const, Gl_imageFormat const, Rectangle2d const *)
{
	__debugbreak();
	return true;
}

// ======================================================================

void CrashReportInformation::addStaticText(char const *, ...)
{
	__debugbreak();
}


// ======================================================================

PerformanceTimer::PerformanceTimer()
{
	__debugbreak();
}

PerformanceTimer::~PerformanceTimer()
{
	__debugbreak();
}

void PerformanceTimer::start()
{
	__debugbreak();
}

void PerformanceTimer::resume()
{
	__debugbreak();
}

void PerformanceTimer::stop()
{
	__debugbreak();
}

float PerformanceTimer::getElapsedTime() const
{
	__debugbreak();
	return 0.0f;
}

// ======================================================================

Transform const & Object::getTransform_o2w() const
{
	__debugbreak();
	return Transform::identity;
}

// ======================================================================

BOOL APIENTRY DllMain(HANDLE, DWORD, LPVOID)
{
	return TRUE;
}
