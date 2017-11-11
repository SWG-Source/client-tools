// ============================================================================
//
// PlotterManager.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "clientGame/FirstClientGame.h"
#include "clientGame/PlotterManager.h"

#if PRODUCTION == 0

#include "clientGame/Game.h"
#include "clientGame/Plotter.h"
#include "clientUserInterface/CuiTextManager.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedDebug/PerformanceTimer.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedMath/VectorArgb.h"
#include "sharedMemoryManager/MemoryManager.h"
#include "sharedNetwork/NetworkHandler.h"

// ============================================================================
//
// PlotterManagerNamespace
//
// ============================================================================

namespace PlotterManagerNamespace
{
	bool      s_installed = false;
	int const s_frameX = 4;
	int const s_frameY = 200;
	int const s_frameWidth = 200;
	int const s_frameHeight = 40;

	bool      s_allEnabled = false;

	Plotter   s_fpsPlotter;
	bool      s_fpsEnabled = false;

	Plotter   s_fileOpenPlotter;
	bool      s_fileOpenEnabled = false;

	Plotter   s_fileSizePlotter;
	bool      s_fileSizeEnabled = false;

	Plotter   s_networkReceivedCompressedPlotter;
	Plotter   s_networkReceivedUncompressedPlotter;
	Plotter   s_lastSecondnetworkReceivedCompressedPlotter;
	Plotter   s_lastSecondnetworkReceivedUncompressedPlotter;
	bool      s_networkReceivedEnabled = false;

	Plotter   s_memoryAllocationsPlotter;
	Plotter   s_memorySizePlotter;
	bool      s_memoryEnabled = false;

	CuiTextManagerTextEnqueueInfo s_font;

	void printText(int const x, int const y, char const * const text);
}

using namespace PlotterManagerNamespace;

//-----------------------------------------------------------------------------
void PlotterManagerNamespace::printText(int const x, int const y, char const * const text)
{
	s_font.textSize = 0.75f;
	s_font.backgroundOpacity = 0.0f;
	s_font.screenVect.x = static_cast<float>(x);
	s_font.screenVect.y = static_cast<float>(y);
	s_font.textColor = UIColor(255, 255, 255);
	CuiTextManager::enqueueText(Unicode::narrowToWide(text), s_font);
}

// ============================================================================
//
// PlotterManager
//
// ============================================================================

//-----------------------------------------------------------------------------
void PlotterManager::install()
{
	InstallTimer const installTimer("PlotterManager::install");

	DEBUG_FATAL(s_installed, ("Already installed"));

	DebugFlags::registerFlag(s_allEnabled, "ClientGame/Plotter", "0_all");
	DebugFlags::registerFlag(s_fpsEnabled, "ClientGame/Plotter", "1_fps");
	DebugFlags::registerFlag(s_fileOpenEnabled, "ClientGame/Plotter", "2_fileOpen");
	DebugFlags::registerFlag(s_fileSizeEnabled, "ClientGame/Plotter", "3_fileSize");
	DebugFlags::registerFlag(s_networkReceivedEnabled, "ClientGame/Plotter", "4_networkReceived");
	DebugFlags::registerFlag(s_memoryEnabled, "ClientGame/Plotter", "5_memory");

	ExitChain::add(&remove, "PlotterManager::remove");

	s_installed = true;
}

//-----------------------------------------------------------------------------
void PlotterManager::remove()
{
	DEBUG_FATAL(!s_installed, ("Not installed"));

	DebugFlags::unregisterFlag(s_allEnabled);
	DebugFlags::unregisterFlag(s_fpsEnabled);
	DebugFlags::unregisterFlag(s_fileOpenEnabled);
	DebugFlags::unregisterFlag(s_fileSizeEnabled);
	DebugFlags::unregisterFlag(s_networkReceivedEnabled);
	DebugFlags::unregisterFlag(s_memoryEnabled);
}

//-----------------------------------------------------------------------------
void PlotterManager::draw()
{
	int y = s_frameY;

	if (s_fpsEnabled || s_allEnabled)
	{
		float const min = 0.0f;
		float const max = 30.0f;
		s_fpsPlotter.setFrame(s_frameX, y, s_frameWidth, s_frameHeight);
		s_fpsPlotter.setColor(VectorArgb::solidRed);
		s_fpsPlotter.setRange(min, max);

		// Frames per second

		static PerformanceTimer fpsTimer;
		fpsTimer.stop();
		float const deltaTime = fpsTimer.getElapsedTime();
		fpsTimer.start();

		if (deltaTime > 0.0f)
		{
			s_fpsPlotter.addValue(1.0f / deltaTime);
		}

		s_fpsPlotter.draw();
		y += s_frameHeight;

		printText(s_frameX + s_frameWidth + 18, y - 6, "fps");

		char text[256];
		snprintf(text, sizeof(text), "%.0f", max);
		printText(s_frameX + s_frameWidth + 11, y - 23, text);
	}

	if (s_fileOpenEnabled || s_allEnabled)
	{
		float const min = 0.0f;
		float const max = 30.0f;
		s_fileOpenPlotter.setFrame(s_frameX, y, s_frameWidth, s_frameHeight);
		s_fileOpenPlotter.setColor(VectorArgb::solidYellow);
		s_fileOpenPlotter.setRange(min, max);

		// Treefile file opens

		static int previousNumberOfFilesOpenedTotal = 0;
		s_fileOpenPlotter.addValue(static_cast<float>(TreeFile::getNumberOfFilesOpenedTotal() - previousNumberOfFilesOpenedTotal));
		previousNumberOfFilesOpenedTotal = TreeFile::getNumberOfFilesOpenedTotal();

		s_fileOpenPlotter.draw();
		y += s_frameHeight;

		printText(s_frameX + s_frameWidth + 34, y - 6, "file opens");

		char text[256];
		snprintf(text, sizeof(text), "%.0f", max);
		printText(s_frameX + s_frameWidth + 11, y - 23, text);
	}

	if (s_fileSizeEnabled || s_allEnabled)
	{
		float const min = 0.0f;
		float const max = 1024.0f * 1024.0f;
		s_fileSizePlotter.setFrame(s_frameX, y, s_frameWidth, s_frameHeight);
		s_fileSizePlotter.setColor(VectorArgb::solidCyan);
		s_fileSizePlotter.setRange(min, max);

		// Treefile file bytes opened

		static int previousSizeOfFilesOpenedTotal = 0;
		s_fileSizePlotter.addValue(static_cast<float>(TreeFile::getSizeOfFilesOpenedTotal() - previousSizeOfFilesOpenedTotal));
		previousSizeOfFilesOpenedTotal = TreeFile::getSizeOfFilesOpenedTotal();

		s_fileSizePlotter.draw();
		y += s_frameHeight;

		printText(s_frameX + s_frameWidth + 29, y - 6, "file size");

		char text[256];
		snprintf(text, sizeof(text), "%.0f MB", max / 1024.0f / 1024.0f);
		printText(s_frameX + s_frameWidth + 17, y - 23, text);
	}

	if (s_networkReceivedEnabled || s_allEnabled)
	{
		float const min = 0.0f;
		float const max = 5.0f * 1024.0f;

		// Uncompressed	received network per frame

		s_networkReceivedUncompressedPlotter.setFrame(s_frameX, y, s_frameWidth, s_frameHeight);
		s_networkReceivedUncompressedPlotter.setColor(VectorArgb(1.0f, 0.4f, 0.4f, 0.4f));
		s_networkReceivedUncompressedPlotter.setRange(min, max);

		static int previousTotalUncompressedByteCount = 0;
		int const uncompressedBytesThisFrame = NetworkHandler::getRecvTotalUncompressedByteCount() - previousTotalUncompressedByteCount;
		s_networkReceivedUncompressedPlotter.addValue(static_cast<float>(uncompressedBytesThisFrame));
		previousTotalUncompressedByteCount = NetworkHandler::getRecvTotalUncompressedByteCount();

		// Compressed received network per frame

		s_networkReceivedCompressedPlotter.setFrame(s_frameX, y, s_frameWidth, s_frameHeight);
		s_networkReceivedCompressedPlotter.setColor(VectorArgb::solidMagenta);
		s_networkReceivedCompressedPlotter.setRange(min, max);

		static int previousTotalCompressedByteCount = 0;
		int const compressedBytesThisFrame = NetworkHandler::getRecvTotalCompressedByteCount() - previousTotalCompressedByteCount;
		s_networkReceivedCompressedPlotter.addValue(static_cast<float>(compressedBytesThisFrame));
		previousTotalCompressedByteCount = NetworkHandler::getRecvTotalCompressedByteCount();

		s_networkReceivedUncompressedPlotter.draw();
		s_networkReceivedCompressedPlotter.draw();
		y += s_frameHeight;

		printText(s_frameX + s_frameWidth + 72, y - 6, "bytes received per frame");
		char text[256];
		snprintf(text, sizeof(text), "%.0f K", max / 1024.0f);
		printText(s_frameX + s_frameWidth + 14, y - 23, text);

		// Last second uncompressed received network per frame

		s_lastSecondnetworkReceivedUncompressedPlotter.setFrame(s_frameX, y, s_frameWidth, s_frameHeight);
		s_lastSecondnetworkReceivedUncompressedPlotter.setColor(VectorArgb(1.0f, 0.4f, 0.4f, 0.4f));
		s_lastSecondnetworkReceivedUncompressedPlotter.setRange(min, max);
		s_lastSecondnetworkReceivedUncompressedPlotter.addValue(static_cast<float>(Game::getReceivedUncompressedBytesPerSecond()));

		// Last second compressed received network per frame

		s_lastSecondnetworkReceivedCompressedPlotter.setFrame(s_frameX, y, s_frameWidth, s_frameHeight);
		s_lastSecondnetworkReceivedCompressedPlotter.setColor(VectorArgb::solidMagenta);
		s_lastSecondnetworkReceivedCompressedPlotter.setRange(min, max);
		s_lastSecondnetworkReceivedCompressedPlotter.addValue(static_cast<float>(Game::getReceivedCompressedBytesPerSecond()));

		s_lastSecondnetworkReceivedUncompressedPlotter.draw();
		s_lastSecondnetworkReceivedCompressedPlotter.draw();
		y += s_frameHeight;

		printText(s_frameX + s_frameWidth + 76, y - 6, "bytes received per second");
		snprintf(text, sizeof(text), "%.0f K", max / 1024.0f);
		printText(s_frameX + s_frameWidth + 14, y - 23, text);
	}

	if (s_memoryEnabled || s_allEnabled)
	{
		{
			// Memory allocations

			float const min = 0.0f;
			float const max = 50.0f;
			static float timer = 0.0f;

			s_memoryAllocationsPlotter.setFrame(s_frameX, y, s_frameWidth, s_frameHeight);
			s_memoryAllocationsPlotter.setColor(VectorArgb::solidGreen);
			s_memoryAllocationsPlotter.setRange(min, max);

			static int previousMemoryAlocationCount = 0;
			int const memoryAllocationCountThisFrame = MemoryManager::getCurrentNumberOfAllocations() - previousMemoryAlocationCount;
			s_memoryAllocationsPlotter.addValue(static_cast<float>(memoryAllocationCountThisFrame));
			previousMemoryAlocationCount = MemoryManager::getCurrentNumberOfAllocations();

			s_memoryAllocationsPlotter.draw();
			y += s_frameHeight;

			printText(s_frameX + s_frameWidth + 78, y - 6, "non-MBM mem allocations");
			char text[256];
			snprintf(text, sizeof(text), "%.0f", max);
			printText(s_frameX + s_frameWidth + 14, y - 23, text);
		}

		{
			// Memory size allocated

			float const min = 0.0f;
			float const max = 1024.0f * 1024.0f;
			static float timer = 0.0f;

			s_memorySizePlotter.setFrame(s_frameX, y, s_frameWidth, s_frameHeight);
			s_memorySizePlotter.setColor(VectorArgb(1.0f, 1.0f, 0.5f, 0.0f));
			s_memorySizePlotter.setRange(min, max);

			static unsigned long previousMemoryAlocationCount = 0;
			int const memoryAllocationCountThisFrame = MemoryManager::getCurrentNumberOfBytesAllocated() - previousMemoryAlocationCount;
			s_memorySizePlotter.addValue(static_cast<float>(memoryAllocationCountThisFrame));
			previousMemoryAlocationCount = MemoryManager::getCurrentNumberOfBytesAllocated();

			s_memorySizePlotter.draw();
			y += s_frameHeight;

			printText(s_frameX + s_frameWidth + 86, y - 6, "non-MBM mem size allocated");
			char text[256];
			snprintf(text, sizeof(text), "%.0f MB", max / 1024.0f / 1024.0f);
			printText(s_frameX + s_frameWidth + 17, y - 23, text);
		}
	}
}

#endif // PRODUCTION == 0

// ============================================================================
