// ======================================================================
//
// SwgVideoCapture.cpp
// copyright (c) 2009 Sony Online Entertainment
//
// ======================================================================

#include <cassert>
#include "clientGraphics/FirstClientGraphics.h"
#include "clientGraphics/Graphics.h"
#include "clientGraphics/SwgVideoCapture.h"
#include "sharedFoundation/FirstSharedFoundation.h"
#include "AudioCapture/AudioCapture.h"
#include "VideoCapture/VideoCapture.h"
#include "SoeUtil/String.h"

#if PRODUCTION == 0

namespace VideoCapture
{

// SoeUtilMemoryAdapter
// Wraps game defined memory functions for SoeUtil
// new/delete are overridden in MemoryManager.h

class SoeUtilMemoryAdapter : public SoeUtil::MemoryHandler
{
public:
	static void Install();
	virtual ~SoeUtilMemoryAdapter()
	{
	}
	virtual void *OnAlloc(int bytes)
	{
		return new byte[bytes];
	}
	virtual void OnFree(void* p)
	{
		delete[] (byte*)p;
	}
private:
	SoeUtilMemoryAdapter()
	{
	}
	SoeUtilMemoryAdapter(const SoeUtilMemoryAdapter&);
	SoeUtilMemoryAdapter& operator=(const SoeUtilMemoryAdapter&);
};


void SoeUtilMemoryAdapter::Install()
{
	static SoeUtilMemoryAdapter s_memoryAdapter;
	static bool s_firstPass = true;
	if(s_firstPass)
	{
		SoeUtil::SetGlobalMemoryHandler(&s_memoryAdapter);
		s_firstPass = false;
	}
}

// RunOnce

void install()
{
	static bool s_firstPass = true;
	if(s_firstPass)
	{
		SoeUtilMemoryAdapter::Install();
		s_firstPass = false;
	}
}

// SwgVideoCaptureProps:
// Transforms SWG props to EncoderProps

class SwgVideoCaptureProps
{
public:
	typedef Smart::SmartPtrT<SwgVideoCaptureProps> Ptr;
	enum
	{
		cMinQuality = 0, // Actual min is 1, 0 means default
		cMaxQuality = 100
	};
	static const EncRes s_defaultRes;
	static const int s_ignoreEncRes;
	static const int s_ignoreDstSec;
	static const char* s_ignoreDstFile;
	static const int s_ignoreDstRate;
	static const int s_ignoreDstQuality;
	SwgVideoCaptureProps();
	~SwgVideoCaptureProps();
	int SrcWidth() const
	{
		return m_srcWidth;
	}
	int SrcHeight() const
	{
		return m_srcHeight;
	}
	EncRes Res() const
	{
		return m_encRes;
	}
	const char* DstFile() const
	{
		return m_dstFile.AsRead();
	}
	int DstWidth() const
	{
		return m_dstWidth;
	}
	int DstHeight() const
	{
		return m_dstHeight;
	}
	int DstQuality() const // cMinQuality-cMaxQuality, 0 means default
	{
		return m_dstQuality;
	}
	bool Config(int srcWidth, // Video frame width in pixels, Required
				int srcHeight, // Video frame height in pixels, Required
				int srcSamplesPerSec, // Audio sample rate, Optional, zero disables audio
				int srcBitsPerSample, // Audio bits per sample, Required if srcSamplePerSec nonzero
				int srcChannels, // Audio channel count, Required if srcSamplePerSec nonzero
				int encRes, // Video encoder output resolution, Optional, ignore zero
				int dstSec, // Encoder run time limit, Optional, ignore neg
				const char* dstFile, // Output file, Optional, ignore null or empty
				int dstRate, // Video frames per sec, Optional, ignore zero
				int dstQuality); // Compression quality, Optional, ignore neg
	bool ReConfig(int srcWidth, // Video frame width in pixels, Required
				  int srcHeight, // Video frame height in pixels, Required
				  int srcSamplesPerSec, // Audio sample rate, Optional, zero disables audio
				  int srcBitsPerSample, // Audio bits per sample, Required if srcSamplePerSec nonzero
				  int srcChannels); // Audio channel count, Required if srcSamplePerSec nonzero
	bool Defaults(int srcWidth, // Video frame width in pixels, Required
				  int srcHeight, // Video frame height in pixels, Required
				  int srcSamplesPerSec, // Audio sample rate, Optional, zero disables audio
				  int srcBitsPerSample, // Audio bits per sample, Required if srcSamplePerSec nonzero
				  int srcChannels); // Audio channel count, Required if srcSamplePerSec nonzero
	bool Validate() const;
	EncoderProps ToEncoderProps() const;
	EncoderLimits ToEncoderLimits() const;
private:
	SourceFmtVid m_srcFmtVid;
	int m_srcWidth;
	int m_srcHeight;
	SourceFmtAud m_srcFmtAud;
	SampleRateAud m_srcSampleRate;
	EncRes m_encRes;
	int m_dstSec;
	SoeUtil::String m_dstFile;
	mutable int m_dstWidth;
	mutable int m_dstHeight;
	int m_dstRate;
	int m_dstScale;
	int m_dstQuality;
};

const EncRes SwgVideoCaptureProps::s_defaultRes = cEncResMed;
const int SwgVideoCaptureProps::s_ignoreEncRes = 0;
const int SwgVideoCaptureProps::s_ignoreDstSec = -1;
const char* SwgVideoCaptureProps::s_ignoreDstFile = 0;
const int SwgVideoCaptureProps::s_ignoreDstRate = 0;
const int SwgVideoCaptureProps::s_ignoreDstQuality = -1;

static const char s_defaultDstFile[] = "VideoCaptureOutput.avi";
static const int s_defaultDstMaxFileGBytes = 100; // 100GB
static const int s_defaultDstRate = 30; // fps
static const int s_defaultDstSec= 0;
static const int s_defaultDstQuality = 0;
static const int s_defaultDstMsecPerStatus = 100; // msec
static const int s_minPcntMemoryAvailable = 20; // %
static const int s_minDiskMBytesAvailable = 200; // MBytes

SwgVideoCaptureProps::SwgVideoCaptureProps():
	m_srcFmtVid(cSourceFmtVidRGB32),
	m_srcWidth(0),
	m_srcHeight(0),
	m_srcFmtAud(cSourceFmtAudInvalid),
	m_srcSampleRate(cSampleRateAudInvalid),
	m_encRes(s_defaultRes),
	m_dstSec(s_defaultDstSec),
	m_dstFile(s_defaultDstFile),
	m_dstWidth(0),
	m_dstHeight(0),
	m_dstRate(s_defaultDstRate),
	m_dstScale(1),
	m_dstQuality(s_defaultDstQuality)
{
	const EncoderRes& encoderRes = EncoderRes::GetEncoderRes(static_cast<EncRes>(m_encRes));
	m_dstWidth = encoderRes.Width();
	m_dstHeight = encoderRes.Height();
}

SwgVideoCaptureProps::~SwgVideoCaptureProps()
{
}

bool SwgVideoCaptureProps::Config(int srcWidth, // Required
								  int srcHeight, // Required
								  int srcSamplesPerSec, // Optional, zero disables audio
								  int srcBitsPerSample, // Required if srcSamplePerSec nonzero
								  int srcChannels, // Required if srcSamplePerSec nonzero
								  int encRes, // Optional, ignore zero
								  int dstSec, // Optional, ignore neg
								  const char* dstFile, // Optional, ignore null or empty
								  int dstRate, // Optional, ignore zero
								  int dstQuality) // Optional, ignore neg, range 0-100, will be remapped below
{
	if((srcWidth <= 0) || (srcHeight <= 0)) // Required
	{
		return false;
	}
	SourceFmtAud srcFmtAud = cSourceFmtAudInvalid;
	SampleRateAud srcSampleRate = cSampleRateAudInvalid;
	if(srcSamplesPerSec) // Optional, zero disables audio
	{
		srcFmtAud = SourceConfigAud2SourceFmtAud(srcBitsPerSample, srcChannels);
		if(!ValidateSourceFmtAud(srcFmtAud))
		{
			return false;
		}
		srcSampleRate = SamplesPerSec2SampleRateAud(srcSamplesPerSec);
		if(!ValidateSampleRateAud(srcSampleRate))
		{
			return false;
		}
	}
	if(s_ignoreEncRes < encRes) // Optional, ignore zero
	{
		if(!ValidateEncRes(encRes))
		{
			return false;
		}
	}
	if(s_ignoreDstRate < dstRate) // Optional, ignore zero
	{
		if(!ValidateRate(dstRate))
		{
			return false;
		}
	}
	if(s_ignoreDstQuality < dstQuality) // Optional, ignore neg
	{
		if(cMaxQuality < dstQuality)
		{
			return false;
		}
	}
	// Success
	m_srcWidth = srcWidth;
	m_srcHeight = srcHeight;
	m_srcFmtAud = srcFmtAud;
	m_srcSampleRate = srcSampleRate;
	if(s_ignoreEncRes < encRes) // Optional, ignore zero
	{
		m_encRes = static_cast<EncRes>(encRes);
	}
	// Always reconfirm res
	const EncoderRes& encoderRes = FindBestEncRes(srcWidth, srcHeight, m_encRes);
	m_encRes = encoderRes.Res();
	m_dstWidth = encoderRes.Width();
	m_dstHeight = encoderRes.Height();
	if(s_ignoreDstSec < dstSec) // Optional, ignore neg
	{
		m_dstSec = dstSec; // Zero dstSec means no timeout
	}
	if(dstFile && *dstFile) // Optional, ignore null or empty
	{
		m_dstFile = dstFile;
	}
	if(s_ignoreDstRate < dstRate) // Optional, ignore zero
	{
		m_dstRate = dstRate;
	}
	if(s_ignoreDstQuality < dstQuality) // Optional, ignore neg
	{
		m_dstQuality = dstQuality;
	}
	return true;
}

bool SwgVideoCaptureProps::ReConfig(int srcWidth, // Required
									int srcHeight, // Required
									int srcSamplesPerSec, // Optional, zero disables audio
									int srcBitsPerSample, // Required if srcSamplePerSec nonzero
									int srcChannels) // Required if srcSamplePerSec nonzero
{
	// Always reconfirm res
	const EncoderRes& encoderRes = FindBestEncRes(srcWidth, srcHeight, m_encRes);
	SourceFmtAud srcFmtAud = cSourceFmtAudInvalid;
	SampleRateAud srcSampleRate = cSampleRateAudInvalid;
	if(srcSamplesPerSec) // Optional, zero disables audio
	{
		srcFmtAud = SourceConfigAud2SourceFmtAud(srcBitsPerSample, srcChannels);
		if(!ValidateSourceFmtAud(srcFmtAud))
		{
			return false;
		}
		srcSampleRate = SamplesPerSec2SampleRateAud(srcSamplesPerSec);
		if(!ValidateSampleRateAud(srcSampleRate))
		{
			return false;
		}
	}
	// Success
	m_srcWidth = srcWidth;
	m_srcHeight = srcHeight;
	m_srcFmtAud = srcFmtAud;
	m_srcSampleRate = srcSampleRate;
	m_encRes = encoderRes.Res();
	m_dstWidth = encoderRes.Width();
	m_dstHeight = encoderRes.Height();
	return true;
}

bool SwgVideoCaptureProps::Defaults(int srcWidth, // Required
									int srcHeight, // Required
									int srcSamplesPerSec, // Optional, zero disables audio
									int srcBitsPerSample, // Required if srcSamplePerSec nonzero
									int srcChannels) // Required if srcSamplePerSec nonzero
{
	const EncoderRes& encoderRes = FindBestEncRes(srcWidth, srcHeight, s_defaultRes);
	SourceFmtAud srcFmtAud = cSourceFmtAudInvalid;
	SampleRateAud srcSampleRate = cSampleRateAudInvalid;
	if(srcSamplesPerSec) // Optional, zero disables audio
	{
		srcFmtAud = SourceConfigAud2SourceFmtAud(srcBitsPerSample, srcChannels);
		if(!ValidateSourceFmtAud(srcFmtAud))
		{
			return false;
		}
		srcSampleRate = SamplesPerSec2SampleRateAud(srcSamplesPerSec);
		if(!ValidateSampleRateAud(srcSampleRate))
		{
			return false;
		}
	}
	// Success
	m_srcWidth = srcWidth;
	m_srcHeight = srcHeight;
	m_srcFmtAud = srcFmtAud;
	m_srcSampleRate = srcSampleRate;
	m_encRes = encoderRes.Res();
	m_dstWidth = encoderRes.Width();
	m_dstHeight = encoderRes.Height();
	m_dstSec = s_defaultDstSec;
	m_dstFile = s_defaultDstFile;
	m_dstRate = s_defaultDstRate;
	m_dstQuality = s_defaultDstQuality;
	return true;
}

bool SwgVideoCaptureProps::Validate() const
{
	if(!ValidateSourceFmtVid(m_srcFmtVid))
	{
		return false;
	}
	// m_dstSec = 0 means no time limit
	if((m_srcWidth <= 0) || (m_srcHeight <= 0) || (m_dstSec < 0) || (m_dstRate <= 0) || (m_dstScale <= 0) || (m_dstWidth <= 0) || (m_dstHeight <= 0))
	{
		return false;
	}
	if(!ValidateEncRes(m_encRes))
	{
		return false;
	}
	if(!ValidateRate(m_dstRate))
	{
		return false;
	}
	if((m_dstQuality < cMinQuality) || (cMaxQuality < m_dstQuality))
	{
		return false;
	}
	if(!m_dstFile.Length())
	{
		return false;
	}
	if((m_srcWidth < m_dstWidth) || (m_srcHeight < m_dstHeight))
	{
		return false;
	}
	return true;
}

EncoderProps SwgVideoCaptureProps::ToEncoderProps() const
{
	// EncoderProps
	EncoderProps encProps;
	encProps.SrcFmtVid(m_srcFmtVid);
	encProps.SrcScan(cSourceScanTopBottom);
	encProps.Res(m_encRes);
	encProps.SrcWidth(m_srcWidth);
	encProps.SrcHeight(m_srcHeight);
	encProps.SrcFmtAud(m_srcFmtAud);
	encProps.SrcSampleRateAud(m_srcSampleRate);
	encProps.DstFmt(cDestFmtAVI);
	encProps.DstMaxFileGBytes(s_defaultDstMaxFileGBytes);
	encProps.DstRate(m_dstRate);
	encProps.DstScale(m_dstScale);
	encProps.DstMsecPerStatus(s_defaultDstMsecPerStatus);
	// Maps (cMinQuality-cMaxQuality) to (c_minQuality - c_maxQuality)
	const int dstQuality = LinearTransform(m_dstQuality,
										   SwgVideoCaptureProps::cMinQuality,
										   SwgVideoCaptureProps::cMaxQuality,
										   c_minQuality,
										   c_maxQuality);
	encProps.DstQuality(dstQuality); 
	encProps.PreScale(true);
	encProps.SaveFirstFrame(false);
	return encProps;
}

EncoderLimits SwgVideoCaptureProps::ToEncoderLimits() const
{
	// EncoderLimits
	EncoderLimits encLimits;
	encLimits.SecDuration(m_dstSec);
	encLimits.MinPcntMemoryAvailable(s_minPcntMemoryAvailable);
	encLimits.MinDiskMBytesAvailable(s_minDiskMBytesAvailable);
	return encLimits;
}

// SwgAudioCaptureCallbackHandler

class SwgAudioCaptureCallbackHandler : public AudioCapture::ICallback
{
public:
	typedef Smart::SmartPtrT<SwgAudioCaptureCallbackHandler> Ptr;
	SwgAudioCaptureCallbackHandler(VideoCapture::VideoCaptureManager* pVideoCaptureManager):
		m_pVideoCaptureManager(pVideoCaptureManager),
		m_pVideoCaptureManager_SingleUse(0)
	{
	}
	SwgAudioCaptureCallbackHandler(VideoCapture::VideoCaptureManager_SingleUse* pVideoCaptureManager):
		m_pVideoCaptureManager(0),
		m_pVideoCaptureManager_SingleUse(pVideoCaptureManager)
	{
	}
	virtual ~SwgAudioCaptureCallbackHandler()
	{
	}
	// ICallback
	virtual bool GetBuffer(CaptureCommon::IBuffer::Ptr& pBuffer)
	{
		if(m_pVideoCaptureManager)
		{
			return m_pVideoCaptureManager->GetAudioBuffer(pBuffer);
		}
		return m_pVideoCaptureManager_SingleUse->GetAudioBuffer(pBuffer);
	}
	virtual void PutBuffer(CaptureCommon::IBuffer::Ptr& pBuffer)
	{
		if(m_pVideoCaptureManager)
		{
			m_pVideoCaptureManager->PutAudioBuffer(pBuffer);
			return;
		}
		m_pVideoCaptureManager_SingleUse->PutAudioBuffer(pBuffer);
	}
private:
	SwgAudioCaptureCallbackHandler(const SwgAudioCaptureCallbackHandler&);
	SwgAudioCaptureCallbackHandler& operator=(const SwgAudioCaptureCallbackHandler&);
	VideoCapture::VideoCaptureManager* m_pVideoCaptureManager;
	VideoCapture::VideoCaptureManager_SingleUse* m_pVideoCaptureManager_SingleUse;
};

// SingleUse

namespace SingleUse
{

VideoCaptureManager_SingleUse::Ptr s_pVideoCaptureSingleUse;
SwgAudioCaptureCallbackHandler::Ptr s_pSwgAudioCaptureCallbackHandler;
VideoCapture::SingleUse::ICallback* s_pVideoCaptureCallback = 0;
AudioCapture::IManager* s_pAudioCaptureManager = 0;
bool s_firstPass = false;

static SwgVideoCaptureProps& getVideoCaptureProps()
{
	static SwgVideoCaptureProps s_videoCaptureProps;
	return s_videoCaptureProps;
}

void config(int resolution, int seconds, int quality, const char* filename, AudioCapture::IManager* pAudioCaptureManager)
{
	SwgVideoCaptureProps& videoCaptureProps = getVideoCaptureProps();
	SwgVideoCaptureProps tmp(videoCaptureProps);
	int samplesPerSec = 0;
	int bitsPerSample = 0;
	int channels = 0;
	if(pAudioCaptureManager)
	{
		pAudioCaptureManager->GetConfig(samplesPerSec, bitsPerSample, channels);
	}
	if(!tmp.Config(Graphics::getCurrentRenderTargetWidth(),
					Graphics::getCurrentRenderTargetHeight(),
					samplesPerSec,
					bitsPerSample,
					channels,
					resolution,
					seconds,
					filename,
					SwgVideoCaptureProps::s_ignoreDstRate,
					quality))
	{
		return;
	}
	if(!tmp.Validate())
	{
		return;
	}
	videoCaptureProps = tmp;
}

void start(VideoCapture::SingleUse::ICallback* pVideoCaptureCallback, AudioCapture::IManager* pAudioCaptureManager)
{
	if(s_pVideoCaptureSingleUse) // In defense of invalid slash calls
	{
		return;
	}
	SwgVideoCaptureProps& videoCaptureProps = getVideoCaptureProps();
	if(!videoCaptureProps.Validate())
	{
		int samplesPerSec = 0;
		int bitsPerSample = 0;
		int channels = 0;
		if(pAudioCaptureManager)
		{
			pAudioCaptureManager->GetConfig(samplesPerSec, bitsPerSample, channels);
		}
		if(!videoCaptureProps.Defaults(Graphics::getCurrentRenderTargetWidth(),
										Graphics::getCurrentRenderTargetHeight(),
										samplesPerSec,
										bitsPerSample,
										channels))
		{
			return;
		}
	}
	VideoCapture::VideoCaptureManager_SingleUse::Ptr pVideoCaptureManager(new VideoCapture::VideoCaptureManager_SingleUse);
	if(!pVideoCaptureManager)
	{
		return;
	}
	SwgAudioCaptureCallbackHandler::Ptr pSwgAudioCaptureCallbackHandler(new SwgAudioCaptureCallbackHandler(pVideoCaptureManager.Ptr()));
	if(!pSwgAudioCaptureCallbackHandler)
	{
		return;
	}
	if(!pVideoCaptureManager->Start(videoCaptureProps.ToEncoderProps(), videoCaptureProps.ToEncoderLimits(), videoCaptureProps.DstFile()))
	{
		return;
	}
	if(pAudioCaptureManager)
	{
		pAudioCaptureManager->Start(pSwgAudioCaptureCallbackHandler.Ptr());
	}
	// Success
	s_pVideoCaptureSingleUse.Attach(pVideoCaptureManager.Detach());
	s_pSwgAudioCaptureCallbackHandler.Attach(pSwgAudioCaptureCallbackHandler.Detach());
	s_pVideoCaptureCallback = pVideoCaptureCallback;
	s_pAudioCaptureManager = pAudioCaptureManager;
	Graphics::createVideoBuffers(videoCaptureProps.DstWidth(), videoCaptureProps.DstHeight());
	s_firstPass = true;
}

void stop()
{
	if(!s_pVideoCaptureSingleUse) // In defense of invalid slash calls
	{
		return;
	}
	if(s_pAudioCaptureManager)
	{
		s_pAudioCaptureManager->Stop();
		s_pAudioCaptureManager = 0;
	}
	s_pVideoCaptureSingleUse->Stop();
}

void run()
{
	if(s_pVideoCaptureSingleUse)
	{
		// Check if frame buffer size changed
		//if(pDevice->IsLost() || !IsVideoCaptureConfigStillValid())
		//{
		//    StopVideoCapture();
		//    return;
		//}
		const VideoCaptureManager_SingleUse::State state = s_pVideoCaptureSingleUse->GiveTime();
		if(VideoCaptureManager_SingleUse::cStateStarted == state)
		{
			// Frame buffer is ready
			if(s_firstPass)
			{
				if(s_pVideoCaptureCallback)
				{
					s_pVideoCaptureCallback->OnStart();
				}
			}
			else
			{
				CaptureCommon::IBuffer::Ptr pBuffer;
				if (!s_pVideoCaptureSingleUse->GetVideoBuffer(pBuffer))
				{
					return;
				}
				const uint32 dataSize = pBuffer->MaxSize();
				if (!Graphics::getVideoBufferData(pBuffer->Data(), dataSize))
				{
					return;
				}
				pBuffer->Size(dataSize);
				if (!s_pVideoCaptureSingleUse->PutVideoBuffer(pBuffer))
				{
					return;
				}
			}
			// Prepare next frame buffer
			Graphics::fillVideoBuffers();
			s_firstPass = false;
			return;
		}
		if((VideoCaptureManager_SingleUse::cStateStopped == state) || (VideoCaptureManager_SingleUse::cStateDead == state))
		{
			// In case encoder stopped by itself
			if(s_pAudioCaptureManager)
			{
				s_pAudioCaptureManager->Stop();
				s_pAudioCaptureManager = 0;
			}
		}
		if(VideoCaptureManager_SingleUse::cStateDead == state)
		{
			if(s_pVideoCaptureCallback)
			{
				s_pVideoCaptureCallback->OnStop();
				s_pVideoCaptureCallback = 0;
			}
			s_pVideoCaptureSingleUse.Release();
			s_pSwgAudioCaptureCallbackHandler.Release();
			Graphics::releaseVideoBuffers();
			return;
		}
	}
}

} // SingleUse

} // VideoCapture

#endif // PRODUCTION
