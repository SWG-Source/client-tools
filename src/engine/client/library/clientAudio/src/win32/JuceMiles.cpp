#include "clientAudio/FirstClientAudio.h"

#if defined(SWG_JUCE_BACKEND)

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdarg>
#include <share.h>
#include <thread>

#pragma push_macro("U8")
#pragma push_macro("U16")
#undef U8
#undef U16
#include "../../../../../../external/3rd/library/JUCE-8.0.14/modules/juce_audio_devices/juce_audio_devices.h"
#include "../../../../../../external/3rd/library/JUCE-8.0.14/modules/juce_audio_formats/juce_audio_formats.h"
#pragma pop_macro("U16")
#pragma pop_macro("U8")

namespace JuceMilesNamespace
{
	using FrameIndex = std::uint64_t;

	struct Vector3
	{
		F32 x = 0.0f;
		F32 y = 0.0f;
		F32 z = 0.0f;
	};

	struct SampleState;

	// A Vector3 whose components are written lock-free by the game thread's
	// per-frame setters and read by the device callback. Components load/store
	// relaxed and independently; a block mixing from a listener position torn
	// between two consecutive frames is inaudible, a lock hold is not.
	struct AtomicVector3
	{
		AtomicVector3(F32 initialX = 0.0f, F32 initialY = 0.0f, F32 initialZ = 0.0f) : x(initialX), y(initialY), z(initialZ)
		{
		}

		std::atomic<F32> x;
		std::atomic<F32> y;
		std::atomic<F32> z;

		Vector3 get() const
		{
			return {x.load(std::memory_order_relaxed), y.load(std::memory_order_relaxed), z.load(std::memory_order_relaxed)};
		}

		void set(F32 newX, F32 newY, F32 newZ)
		{
			x.store(newX, std::memory_order_relaxed);
			y.store(newY, std::memory_order_relaxed);
			z.store(newZ, std::memory_order_relaxed);
		}
	};

	struct DriverState final : public juce::AudioIODeviceCallback
	{
		HDIGDRIVER handle = nullptr;
		juce::AudioDeviceManager deviceManager;
		juce::Reverb reverb;
		juce::AudioBuffer<float> wetBuffer;
		double sampleRate = 44100.0;
		S32 outputChannels = 2;
		S32 bufferSize = 0;
		MSS_MC_SPEC channelSpec = MSS_MC_STEREO;
		S32 roomType = ENVIRONMENT_GENERIC;
		std::atomic<F32> rolloffFactor = 1.0f;
		std::atomic<F32> masterVolume = 1.0f;
		std::atomic<F32> masterDryLevel = 1.0f;
		std::atomic<F32> masterWetLevel = 1.0f;
		AtomicVector3 listenerPosition;
		AtomicVector3 listenerVelocity;
		AtomicVector3 listenerFront{0.0f, 0.0f, -1.0f};
		AtomicVector3 listenerUp{0.0f, 1.0f, 0.0f};
		std::atomic<bool> deviceRunning = false;
		// Miss masking: the last successfully mixed block, replayed (fading) on a
		// lock-miss so a residual miss is stale audio instead of a silent gap.
		// Touched ONLY by the device callback thread.
		juce::AudioBuffer<float> lastBlock;
		int lastBlockSamples = 0;
		F32 lastBlockRepeatGain = 1.0f;

		void audioDeviceIOCallbackWithContext(
			float const *const *,
			int,
			float *const *outputChannelData,
			int numOutputChannels,
			int numSamples,
			juce::AudioIODeviceCallbackContext const &) override;
		void audioDeviceAboutToStart(juce::AudioIODevice *device) override;
		void audioDeviceStopped() override;
		void audioDeviceError(juce::String const &message) override;
	};

	struct SampleState
	{
		explicit SampleState(HDIGDRIVER driverHandle) : driver(driverHandle)
		{
		}

		HDIGDRIVER driver = nullptr;
		juce::AudioBuffer<float> pcm;
		// Streamed assets: the pcm buffer is allocated FULL SIZE (zeroed) at open with the
		// metadata already correct from the header; a worker fills it in chunks behind the
		// play cursor. fillToken guards against release-or-reassign mid-fill.
		std::uint64_t fillToken = 0;
		std::vector<U8> encodedData;
		FrameIndex totalFrames = 0;
		FrameIndex seekFrame = 0;
		double cursorFrame = 0.0;
		S32 sourceChannels = 0;
		S32 sourceSampleRate = 0;
		S32 sourceBits = 0;
		S32 encodedBlockAlign = 1;
		// The atomics below are the per-frame HOT mix parameters: the engine sets
		// them per sound per frame, and every lock those setters held was a chance
		// for the device callback's try_to_lock to miss (one audible pop each --
		// the ~0.13%-of-blocks drip measured 2026-08-15). They are written WITHOUT
		// s_mutex (relaxed stores from the game thread) and snapshotted per block
		// by the callback. Everything structural stays under s_mutex.
		std::atomic<S32> playbackRate = 0;
		std::atomic<F32> leftVolume = 1.0f;
		std::atomic<F32> rightVolume = 1.0f;
		std::atomic<F32> dryLevel = 1.0f;
		std::atomic<F32> wetLevel = 0.0f;
		std::atomic<F32> obstruction = 0.0f;
		std::atomic<F32> occlusion = 0.0f;
		AtomicVector3 position;
		AtomicVector3 velocity;
		std::atomic<F32> minDistance = 1.0f;
		std::atomic<F32> maxDistance = 100.0f;
		F32 filterLeft = 0.0f;
		F32 filterRight = 0.0f;
		S32 loopCount = 1;
		S32 remainingLoops = 1;
		S32 loopStartOffset = 0;
		S32 loopEndOffset = -1;
		bool loopActive = false;
		AILSAMPLECB eosCallback = nullptr;
		HSTREAM ownerStream = nullptr;
		std::atomic<bool> completed = false;
		std::atomic<U32> status = SMP_DONE;
		std::atomic<bool> spatialized = false;
	};

	struct StreamState
	{
		HSAMPLE sample = nullptr;
		AILSTREAMCB callback = nullptr;
		std::string filename;
	};

	using DriverMap = std::unordered_map<HDIGDRIVER, std::unique_ptr<DriverState>>;
	using SampleMap = std::unordered_map<HSAMPLE, std::unique_ptr<SampleState>>;
	using StreamMap = std::unordered_map<HSTREAM, std::unique_ptr<StreamState>>;

	// s_mutex guards the handle maps and all structural sample state. The device
	// callback takes it with try_to_lock, and every hold on any other thread is a
	// chance for that to miss (= one masked-or-audible dropout block). The
	// per-frame HOT entry points (volume/rate/3D/status polls) therefore take no
	// lock at all: the engine calls every AIL_ function from the game thread only
	// -- the maps' sole mutator -- so an unlocked map FIND on the game thread
	// cannot race the (still locked) insert/erase paths, and the values written
	// are relaxed atomics: stale by one block is fine, a lock hold is not.
	std::recursive_mutex s_mutex;
	DriverMap s_drivers;
	SampleMap s_samples;
	StreamMap s_streams;
	AIL_file_open_callback s_fileOpenCallback = nullptr;
	AIL_file_close_callback s_fileCloseCallback = nullptr;
	AIL_file_seek_callback s_fileSeekCallback = nullptr;
	AIL_file_read_callback s_fileReadCallback = nullptr;
	std::unordered_map<U32, SINTa> s_preferences;
	std::unique_ptr<juce::ScopedJuceInitialiser_GUI> s_juceInitialiser;
	juce::AudioFormatManager s_formatManager;
	std::string s_redistDirectory;
	std::string s_lastError;
	S32 s_fileError = AIL_NO_ERROR;
	bool s_started = false;

	template <typename T>
	T clampValue(T value, T low, T high)
	{
		return value < low ? low : (value > high ? high : value);
	}

	U16 readU16(U8 const *data)
	{
		return static_cast<U16>(data[0] | (static_cast<U16>(data[1]) << 8));
	}

	U32 readU32(U8 const *data)
	{
		return static_cast<U32>(data[0]) |
			   (static_cast<U32>(data[1]) << 8) |
			   (static_cast<U32>(data[2]) << 16) |
			   (static_cast<U32>(data[3]) << 24);
	}

	void setError(char const *message)
	{
		s_lastError = message ? message : "Unknown JUCE audio error";
	}

	DriverState *findDriver(HDIGDRIVER driver)
	{
		auto const iter = s_drivers.find(driver);
		return iter == s_drivers.end() ? nullptr : iter->second.get();
	}

	SampleState *findSample(HSAMPLE sample)
	{
		auto const iter = s_samples.find(sample);
		return iter == s_samples.end() ? nullptr : iter->second.get();
	}

	StreamState *findStream(HSTREAM stream)
	{
		auto const iter = s_streams.find(stream);
		return iter == s_streams.end() ? nullptr : iter->second.get();
	}

	Vector3 subtract(Vector3 const &a, Vector3 const &b)
	{
		return {a.x - b.x, a.y - b.y, a.z - b.z};
	}

	F32 dot(Vector3 const &a, Vector3 const &b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	Vector3 cross(Vector3 const &a, Vector3 const &b)
	{
		return {
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x};
	}

	F32 length(Vector3 const &value)
	{
		return std::sqrt(dot(value, value));
	}

	Vector3 normalize(Vector3 value, Vector3 const &fallback)
	{
		F32 const magnitude = length(value);
		if (magnitude <= 0.0001f)
			return fallback;
		F32 const inverse = 1.0f / magnitude;
		return {value.x * inverse, value.y * inverse, value.z * inverse};
	}

	bool isWave(U8 const *data, size_t size)
	{
		return size >= 12 && std::memcmp(data, "RIFF", 4) == 0 && std::memcmp(data + 8, "WAVE", 4) == 0;
	}

	bool isOgg(U8 const *data, size_t size)
	{
		return size >= 4 && std::memcmp(data, "OggS", 4) == 0;
	}

	bool isMp3(U8 const *data, size_t size)
	{
		if (size >= 3 && std::memcmp(data, "ID3", 3) == 0)
			return true;
		for (size_t i = 0; i + 1 < (std::min)(size, static_cast<size_t>(4096)); ++i)
		{
			if (data[i] == 0xff && (data[i + 1] & 0xe0) == 0xe0)
				return true;
		}
		return false;
	}

	bool findWaveChunk(U8 const *data, size_t size, char const id[4], size_t &offset, U32 &chunkLength)
	{
		if (!isWave(data, size))
			return false;

		size_t position = 12;
		while (position + 8 <= size)
		{
			U32 const declaredLength = readU32(data + position + 4);
			if (std::memcmp(data + position, id, 4) == 0)
			{
				offset = position + 8;
				chunkLength = static_cast<U32>((std::min)(static_cast<size_t>(declaredLength), size - offset));
				return true;
			}
			position += 8 + declaredLength + (declaredLength & 1u);
		}
		return false;
	}

	// [audio.decode] verification log. _fsopen/_SH_DENYWR so it can be read while the
	// client runs. One line per decode; cheap enough to leave on.
	void audioDecodeLog(char const *format, ...)
	{
		static FILE *s_log = _fsopen("audio-decode.log", "a", _SH_DENYWR);
		if (!s_log)
			return;
		SYSTEMTIME st;
		GetLocalTime(&st);
		fprintf(s_log, "%02d:%02d:%02d.%03d ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
		va_list va;
		va_start(va, format);
		vfprintf(s_log, format, va);
		va_end(va);
		fputc('\n', s_log);
		fflush(s_log);
	}

	std::uint64_t s_nextFillToken = 1;

	// Result of a pure decode: produced WITHOUT touching SampleState, so the expensive
	// work can run outside s_mutex (the device callback plays SILENCE for any block where
	// it cannot take that lock -- a decode held under it is an audible gap).
	struct DecodedAudio
	{
		juce::AudioBuffer<float> pcm;
		S32        channels = 0;
		S32        sampleRate = 0;
		S32        bits = 0;
		S32        blockAlign = 1;
		FrameIndex totalFrames = 0;
	};

	bool decodeEncoded(std::vector<U8> const &encoded, DecodedAudio &out)
	{
		if (encoded.empty())
		{
			setError("Audio data is empty");
			return false;
		}
		auto stream = std::make_unique<juce::MemoryInputStream>(encoded.data(), encoded.size(), false);
		std::unique_ptr<juce::AudioFormatReader> reader(s_formatManager.createReaderFor(std::move(stream)));
		if (!reader || reader->numChannels == 0 || reader->sampleRate <= 0.0 || reader->lengthInSamples <= 0)
		{
			setError("JUCE could not recognize or decode the audio data");
			return false;
		}
		if (reader->lengthInSamples > static_cast<juce::int64>((std::numeric_limits<int>::max)()))
		{
			setError("Audio asset is too large for the in-memory JUCE decoder");
			return false;
		}
		out.channels    = static_cast<S32>((std::min)(reader->numChannels, 2u));
		out.sampleRate  = static_cast<S32>(reader->sampleRate + 0.5);
		out.bits        = static_cast<S32>(reader->bitsPerSample);
		out.totalFrames = static_cast<FrameIndex>(reader->lengthInSamples);
		out.blockAlign  = (std::max)(1, out.channels * (std::max)(1, out.bits / 8));
		if (isWave(encoded.data(), encoded.size()))
		{
			size_t formatOffset = 0;
			U32 formatLength = 0;
			if (findWaveChunk(encoded.data(), encoded.size(), "fmt ", formatOffset, formatLength) && formatLength >= 16)
				out.blockAlign = (std::max)(1, static_cast<S32>(readU16(encoded.data() + formatOffset + 12)));
		}
		out.pcm.setSize(out.channels, static_cast<int>(out.totalFrames), false, true, false);
		if (!reader->read(&out.pcm, 0, static_cast<int>(out.totalFrames), 0, true, out.channels > 1))
		{
			out.pcm.setSize(0, 0);
			setError("JUCE failed while decoding the audio data");
			return false;
		}
		return true;
	}

	// Move a finished decode into the sample. Caller holds s_mutex.
	void applyDecoded(SampleState &sample, DecodedAudio &decoded)
	{
		sample.sourceChannels    = decoded.channels;
		sample.sourceSampleRate  = decoded.sampleRate;
		sample.sourceBits        = decoded.bits;
		sample.totalFrames       = decoded.totalFrames;
		sample.playbackRate.store(decoded.sampleRate, std::memory_order_relaxed);
		sample.encodedBlockAlign = decoded.blockAlign;
		sample.pcm               = std::move(decoded.pcm);
		sample.seekFrame         = 0;
		sample.cursorFrame       = 0.0;
		sample.completed.store(false, std::memory_order_release);
		sample.status.store(SMP_DONE, std::memory_order_relaxed);
	}


	// Dropout metric: any block where the device callback cannot take s_mutex
	// (try_to_lock below) is a dropout -- masked by replaying the previous block,
	// but still counted, because misses ~ 0 remains the verifiable clean bar.
	// Reported from AIL_serve (never from the callback itself).
	std::atomic<std::uint64_t> s_callbackBlocks{0};
	std::atomic<std::uint64_t> s_callbackLockMisses{0};

	// Streamed open: HEADER-FIRST. The engine snapshots a sample's metadata the moment
	// AIL_open_stream returns -- Audio.cpp:3031 reads playbackRate ONCE into the Sound2 and
	// Sound2d::alter re-applies that captured value EVERY frame -- so metadata that is wrong
	// at open stays wrong forever (a pending-decode rate of 0 renders music at the 0.0625x
	// mixer clamp: four octaves down, measured 2026-08-15). The PCM DATA, by contrast, may
	// arrive late: nothing engine-side abandons a slow stream, and the mixer reads whatever
	// is in the buffer.
	//
	// So: parse the header synchronously (cheap -- no DCT work), set totalFrames/rate/
	// channels/playbackRate correctly, allocate the FULL pcm buffer zeroed, decode the
	// first block inline so play starts with real audio, then fill the rest on a worker.
	// The buffer is never resized after this, so the audio callback's pointers stay
	// valid; the worker decodes each chunk OUTSIDE the lock into scratch and takes the
	// lock only for the copy-in and token check. Worst case on any failure is silence in
	// the un-filled tail -- never wrong metadata, never a state the engine cannot handle.
	bool openStreamedSample(SampleState &sample, HSAMPLE handle, char const *name)
	{
		double const t0 = juce::Time::getMillisecondCounterHiRes();

		auto headerStream = std::make_unique<juce::MemoryInputStream>(sample.encodedData.data(), sample.encodedData.size(), false);
		std::unique_ptr<juce::AudioFormatReader> reader(s_formatManager.createReaderFor(std::move(headerStream)));
		if (!reader || reader->numChannels == 0 || reader->sampleRate <= 0.0 || reader->lengthInSamples <= 0)
			return false;                       // caller falls back to the synchronous path
		if (reader->lengthInSamples > static_cast<juce::int64>((std::numeric_limits<int>::max)()))
			return false;

		//-- metadata: correct BEFORE the engine can look
		sample.sourceChannels    = static_cast<S32>((std::min)(reader->numChannels, 2u));
		sample.sourceSampleRate  = static_cast<S32>(reader->sampleRate + 0.5);
		sample.sourceBits        = static_cast<S32>(reader->bitsPerSample);
		sample.totalFrames       = static_cast<FrameIndex>(reader->lengthInSamples);
		sample.playbackRate.store(sample.sourceSampleRate, std::memory_order_relaxed);
		sample.encodedBlockAlign = (std::max)(1, sample.sourceChannels * (std::max)(1, sample.sourceBits / 8));
		sample.seekFrame         = 0;
		sample.cursorFrame       = 0.0;
		sample.completed.store(false, std::memory_order_release);
		sample.status.store(SMP_DONE, std::memory_order_relaxed);   // normal loaded-not-started, same as the sync path

		//-- full-size buffer, zero-filled; NEVER resized again
		sample.pcm.setSize(sample.sourceChannels, static_cast<int>(sample.totalFrames), false, true, false);

		//-- first block inline so playback opens on real audio, not zeros (~1 second)
		int const firstBlock = static_cast<int>((std::min)(sample.totalFrames, static_cast<FrameIndex>(sample.sourceSampleRate)));
		if (firstBlock > 0 && !reader->read(&sample.pcm, 0, firstBlock, 0, true, sample.sourceChannels > 1))
			return false;

		std::uint64_t const token = s_nextFillToken++;
		sample.fillToken = token;

		double const openMs = juce::Time::getMillisecondCounterHiRes() - t0;
		audioDecodeLog("[audio.stream] open=%.1fms frames=%d ch=%d rate=%d firstBlock=%d name=%s",
			openMs, static_cast<int>(sample.totalFrames), static_cast<int>(sample.sourceChannels),
			static_cast<int>(sample.sourceSampleRate), firstBlock, (name && *name) ? name : "(unnamed)");

		//-- background fill for the remainder. The reader (over a COPY of the encoded
		//   bytes) lives on the worker. The worker decodes the WHOLE track into a
		//   private buffer and swaps it in under the lock in O(1): the previous
		//   per-chunk copy-in held the lock for a 2MB memcpy per chunk, and the
		//   measured dropout bursts sat exactly inside long fills. Decoding from
		//   frame 0 re-does the inline first second redundantly but keeps the buffer
		//   seam-free. The 1s first block covers the swap comfortably: fills measure
		//   55-316ms (decode ~24M frames/s), so only a ~9-minute track could outrun it.
		if (static_cast<FrameIndex>(firstBlock) < sample.totalFrames)
		{
			FrameIndex const total = sample.totalFrames;
			S32 const channels = sample.sourceChannels;
			std::vector<U8> encodedCopy = sample.encodedData;
			std::string nameCopy = (name && *name) ? name : "";
			std::thread([handle, token, total, channels,
			             encodedCopy = std::move(encodedCopy), nameCopy = std::move(nameCopy)]() mutable
			{
				double const w0 = juce::Time::getMillisecondCounterHiRes();
				auto fillStream = std::make_unique<juce::MemoryInputStream>(encodedCopy.data(), encodedCopy.size(), false);
				std::unique_ptr<juce::AudioFormatReader> fillReader(s_formatManager.createReaderFor(std::move(fillStream)));
				if (!fillReader)
					return;                     // tail stays silent; metadata is still correct

				juce::AudioBuffer<float> full(channels, static_cast<int>(total));
				if (!fillReader->read(&full, 0, static_cast<int>(total), 0, true, channels > 1))
					return;                     // keep the inline first block; tail stays silent
				audioDecodeLog("[audio.stream] fill=%.1fms name=%s", juce::Time::getMillisecondCounterHiRes() - w0, nameCopy.c_str());

				std::lock_guard<std::recursive_mutex> lock(s_mutex);
				auto const iter = s_samples.find(handle);
				if (iter == s_samples.end() || iter->second->fillToken != token)
					return;                     // released or reassigned: stop, touch nothing
				iter->second->pcm = std::move(full);   // O(1) pointer move -- the only in-lock work
			}).detach();
		}
		return true;
	}


	FrameIndex mapMp3OffsetToFrame(std::vector<U8> const &data, S32 requestedOffset)
	{
		if (requestedOffset <= 0)
			return 0;

		size_t position = 0;
		if (data.size() >= 10 && std::memcmp(data.data(), "ID3", 3) == 0)
			position = 10 + ((data[6] & 0x7f) << 21) + ((data[7] & 0x7f) << 14) + ((data[8] & 0x7f) << 7) + (data[9] & 0x7f);

		static int const bitrateMpeg1Layer3[16] = {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0};
		static int const bitrateMpeg2Layer3[16] = {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0};
		static int const sampleRates[3] = {44100, 48000, 32000};
		FrameIndex frameCursor = 0;

		while (position + 4 <= data.size())
		{
			if (static_cast<S32>(position) >= requestedOffset)
				return frameCursor;
			U32 const header = (static_cast<U32>(data[position]) << 24) |
							   (static_cast<U32>(data[position + 1]) << 16) |
							   (static_cast<U32>(data[position + 2]) << 8) |
							   static_cast<U32>(data[position + 3]);
			if ((header & 0xffe00000u) != 0xffe00000u)
			{
				++position;
				continue;
			}

			int const versionBits = (header >> 19) & 3;
			int const layerBits = (header >> 17) & 3;
			int const bitrateIndex = (header >> 12) & 15;
			int const sampleRateIndex = (header >> 10) & 3;
			int const padding = (header >> 9) & 1;
			if (versionBits == 1 || layerBits != 1 || bitrateIndex == 0 || bitrateIndex == 15 || sampleRateIndex == 3)
			{
				++position;
				continue;
			}

			bool const mpeg1 = versionBits == 3;
			int sampleRate = sampleRates[sampleRateIndex];
			if (versionBits == 2)
				sampleRate /= 2;
			else if (versionBits == 0)
				sampleRate /= 4;
			int const bitrate = (mpeg1 ? bitrateMpeg1Layer3[bitrateIndex] : bitrateMpeg2Layer3[bitrateIndex]) * 1000;
			int const frameLength = (mpeg1 ? 144 : 72) * bitrate / sampleRate + padding;
			if (frameLength <= 0 || position + static_cast<size_t>(frameLength) > data.size())
			{
				++position;
				continue;
			}
			position += static_cast<size_t>(frameLength);
			frameCursor += mpeg1 ? 1152 : 576;
		}
		return frameCursor;
	}

	FrameIndex mapEncodedOffsetToFrame(SampleState const &sample, S32 offset)
	{
		if (offset <= 0 || sample.totalFrames == 0)
			return 0;
		U8 const *data = sample.encodedData.data();
		size_t const size = sample.encodedData.size();
		if (isWave(data, size))
		{
			size_t dataOffset = 0;
			U32 dataLength = 0;
			if (findWaveChunk(data, size, "data", dataOffset, dataLength))
			{
				std::int64_t const relative = (std::max)(static_cast<std::int64_t>(offset) - static_cast<std::int64_t>(dataOffset), static_cast<std::int64_t>(0));
				return (std::min)(sample.totalFrames, static_cast<FrameIndex>(relative / sample.encodedBlockAlign));
			}
		}
		else if (isMp3(data, size))
		{
			return (std::min)(sample.totalFrames, mapMp3OffsetToFrame(sample.encodedData, offset));
		}
		if (size == 0)
			return 0;
		double const ratio = clampValue(static_cast<double>(offset) / static_cast<double>(size), 0.0, 1.0);
		return static_cast<FrameIndex>(ratio * static_cast<double>(sample.totalFrames));
	}

	U32 mapFrameToEncodedOffset(SampleState const &sample, FrameIndex frame)
	{
		if (sample.totalFrames == 0 || sample.encodedData.empty())
			return 0;
		if (isWave(sample.encodedData.data(), sample.encodedData.size()))
		{
			size_t dataOffset = 0;
			U32 dataLength = 0;
			if (findWaveChunk(sample.encodedData.data(), sample.encodedData.size(), "data", dataOffset, dataLength))
			{
				FrameIndex const byteOffset = static_cast<FrameIndex>(dataOffset) + frame * static_cast<FrameIndex>(sample.encodedBlockAlign);
				return static_cast<U32>((std::min)(byteOffset, static_cast<FrameIndex>((std::numeric_limits<U32>::max)())));
			}
		}
		return static_cast<U32>((std::min)(static_cast<FrameIndex>(sample.encodedData.size()),
										   frame * static_cast<FrameIndex>(sample.encodedData.size()) / sample.totalFrames));
	}

	juce::Reverb::Parameters getReverbPreset(S32 roomType)
	{
		juce::Reverb::Parameters parameters;
		parameters.dryLevel = 0.0f;
		parameters.wetLevel = 1.0f;
		parameters.width = 1.0f;
		parameters.freezeMode = 0.0f;

		switch (roomType)
		{
			case ENVIRONMENT_PADDEDCELL:
				parameters.roomSize = 0.08f;
				parameters.damping = 0.95f;
				break;
			case ENVIRONMENT_LIVINGROOM:
				parameters.roomSize = 0.18f;
				parameters.damping = 0.80f;
				break;
			case ENVIRONMENT_ROOM:
				parameters.roomSize = 0.28f;
				parameters.damping = 0.68f;
				break;
			case ENVIRONMENT_BATHROOM:
				parameters.roomSize = 0.35f;
				parameters.damping = 0.28f;
				break;
			case ENVIRONMENT_HALLWAY:
				parameters.roomSize = 0.48f;
				parameters.damping = 0.58f;
				break;
			case ENVIRONMENT_STONECORRIDOR:
				parameters.roomSize = 0.56f;
				parameters.damping = 0.35f;
				break;
			case ENVIRONMENT_AUDITORIUM:
				parameters.roomSize = 0.68f;
				parameters.damping = 0.48f;
				break;
			case ENVIRONMENT_CONCERTHALL:
				parameters.roomSize = 0.76f;
				parameters.damping = 0.40f;
				break;
			case ENVIRONMENT_CAVE:
				parameters.roomSize = 0.84f;
				parameters.damping = 0.25f;
				break;
			case ENVIRONMENT_HANGAR:
				parameters.roomSize = 0.88f;
				parameters.damping = 0.38f;
				break;
			case ENVIRONMENT_ARENA:
				parameters.roomSize = 0.94f;
				parameters.damping = 0.32f;
				break;
			case ENVIRONMENT_UNDERWATER:
				parameters.roomSize = 0.52f;
				parameters.damping = 0.98f;
				break;
			case ENVIRONMENT_MOUNTAINS:
			case ENVIRONMENT_PLAIN:
			case ENVIRONMENT_FOREST:
			case ENVIRONMENT_CITY:
				parameters.roomSize = 0.32f;
				parameters.damping = 0.72f;
				break;
			default:
				parameters.roomSize = 0.42f;
				parameters.damping = 0.58f;
				break;
		}
		return parameters;
	}

	void applyRoomPreset(DriverState &driver)
	{
		driver.reverb.setParameters(getReverbPreset(driver.roomType));
		driver.reverb.reset();
	}

	S32 requestedChannelCount(S32 channelSpec)
	{
		switch (channelSpec)
		{
			case MSS_MC_MONO:
				return 1;
			case MSS_MC_51_DTS:
			case MSS_MC_51_DISCRETE:
				return 6;
			case MSS_MC_61_DISCRETE:
				return 7;
			case MSS_MC_71_DISCRETE:
				return 8;
			default:
				return 2;
		}
	}

	F32 calculateDoppler(DriverState const &driver, SampleState const &sample, Vector3 const &direction)
	{
		Vector3 const sourceVelocity = sample.velocity.get();
		F32 const speedOfSound = 343.0f;
		F32 const listenerRadial = dot(driver.listenerVelocity.get(), direction);
		F32 const sourceRadial = dot(sourceVelocity, direction);
		F32 const numerator = clampValue(speedOfSound - listenerRadial, speedOfSound * 0.25f, speedOfSound * 4.0f);
		F32 const denominator = clampValue(speedOfSound - sourceRadial, speedOfSound * 0.25f, speedOfSound * 4.0f);
		return clampValue(numerator / denominator, 0.5f, 2.0f);
	}

	void finishSample(SampleState &sample)
	{
		sample.status.store(SMP_DONE, std::memory_order_relaxed);
		sample.completed.store(true, std::memory_order_release);
	}

	void mixSample(
		DriverState &driver,
		SampleState &sample,
		float *const *outputs,
		S32 outputChannels,
		S32 numSamples,
		float *wetLeft,
		float *wetRight)
	{
		if (sample.status.load(std::memory_order_relaxed) != SMP_PLAYING || sample.totalFrames == 0 || sample.pcm.getNumSamples() == 0)
			return;

		//-- the hot parameters are relaxed atomics written lock-free by the game
		//   thread's per-frame setters; snapshot once so the block mixes coherently
		bool const spatialized = sample.spatialized.load(std::memory_order_relaxed);
		F32 const sampleLeftVolume = sample.leftVolume.load(std::memory_order_relaxed);
		F32 const sampleRightVolume = sample.rightVolume.load(std::memory_order_relaxed);
		F32 const sampleDryLevel = sample.dryLevel.load(std::memory_order_relaxed);
		F32 const sampleWetLevel = sample.wetLevel.load(std::memory_order_relaxed);
		F32 const obstruction = sample.obstruction.load(std::memory_order_relaxed);
		F32 const occlusion = sample.occlusion.load(std::memory_order_relaxed);
		F32 const masterVolume = driver.masterVolume.load(std::memory_order_relaxed);
		F32 const masterDryLevel = driver.masterDryLevel.load(std::memory_order_relaxed);
		F32 const masterWetLevel = driver.masterWetLevel.load(std::memory_order_relaxed);
		Vector3 const listenerFront = driver.listenerFront.get();
		Vector3 const listenerUp = driver.listenerUp.get();

		Vector3 const sourcePosition = sample.position.get();
		Vector3 const relative = subtract(sourcePosition, driver.listenerPosition.get());
		F32 const distance = length(relative);
		Vector3 const direction = normalize(relative, listenerFront);
		Vector3 const right = normalize(cross(listenerFront, listenerUp), {1.0f, 0.0f, 0.0f});
		F32 const sidePan = clampValue(dot(direction, right), -1.0f, 1.0f);
		F32 const frontness = clampValue((dot(direction, normalize(listenerFront, {0.0f, 0.0f, -1.0f})) + 1.0f) * 0.5f, 0.0f, 1.0f);

		F32 attenuation = 1.0f;
		if (spatialized)
		{
			F32 const maximumDistance = (std::max)(sample.maxDistance.load(std::memory_order_relaxed), 1.0f);
			F32 const minimumDistance = clampValue(sample.minDistance.load(std::memory_order_relaxed), 0.0f, maximumDistance);
			if (distance >= maximumDistance)
				attenuation = 0.0f;
			else if (distance > minimumDistance && maximumDistance > minimumDistance)
			{
				F32 const normalizedDistance = (distance - minimumDistance) / (maximumDistance - minimumDistance);
				attenuation = std::pow(1.0f - normalizedDistance, (std::max)(driver.rolloffFactor.load(std::memory_order_relaxed), 0.05f));
			}
		}

		F32 const blocked = clampValue((std::max)(obstruction, occlusion), 0.0f, 1.0f);
		F32 const blockedGain = clampValue(1.0f - obstruction * 0.35f - occlusion * 0.20f, 0.0f, 1.0f);
		F32 const cutoff = 20000.0f - blocked * 18500.0f;
		F32 const filterAlpha = std::exp(-2.0f * 3.14159265358979323846f * cutoff / static_cast<F32>((std::max)(driver.sampleRate, 1.0)));
		F32 const panLeft = std::sqrt(0.5f * (1.0f - sidePan));
		F32 const panRight = std::sqrt(0.5f * (1.0f + sidePan));
		F32 const spatialVolume = (sampleLeftVolume + sampleRightVolume) * 0.5f * attenuation * blockedGain * masterVolume;
		F32 const doppler = spatialized ? calculateDoppler(driver, sample, direction) : 1.0f;
		double const step = clampValue(
			static_cast<double>((std::max)(sample.playbackRate.load(std::memory_order_relaxed), 1)) / (std::max)(driver.sampleRate, 1.0) * doppler,
			0.0625,
			8.0);

		float const *sourceLeft = sample.pcm.getReadPointer(0);
		float const *sourceRight = sample.sourceChannels > 1 ? sample.pcm.getReadPointer(1) : sourceLeft;
		FrameIndex const loopStart = (std::min)(mapEncodedOffsetToFrame(sample, sample.loopStartOffset), sample.totalFrames - 1);
		FrameIndex const loopEnd = sample.loopEndOffset < 0
									   ? sample.totalFrames
									   : clampValue(mapEncodedOffsetToFrame(sample, sample.loopEndOffset), loopStart + 1, sample.totalFrames);

		for (S32 outputFrame = 0; outputFrame < numSamples; ++outputFrame)
		{
			if (sample.cursorFrame >= static_cast<double>(sample.totalFrames))
			{
				finishSample(sample);
				break;
			}

			FrameIndex const frame0 = static_cast<FrameIndex>(sample.cursorFrame);
			FrameIndex const frame1 = (std::min)(frame0 + 1, sample.totalFrames - 1);
			F32 const fraction = static_cast<F32>(sample.cursorFrame - static_cast<double>(frame0));
			F32 left = sourceLeft[frame0] + (sourceLeft[frame1] - sourceLeft[frame0]) * fraction;
			F32 rightSample = sourceRight[frame0] + (sourceRight[frame1] - sourceRight[frame0]) * fraction;
			sample.filterLeft = left + filterAlpha * (sample.filterLeft - left);
			sample.filterRight = rightSample + filterAlpha * (sample.filterRight - rightSample);
			left = sample.filterLeft;
			rightSample = sample.filterRight;

			if (spatialized)
			{
				F32 const mono = (left + rightSample) * 0.5f;
				F32 const dryGain = spatialVolume * sampleDryLevel * masterDryLevel;
				F32 const frontGain = outputChannels >= 4 ? std::sqrt(frontness) : 1.0f;
				F32 const rearGain = outputChannels >= 4 ? std::sqrt(1.0f - frontness) : 0.0f;
				if (outputChannels > 0 && outputs[0])
					outputs[0][outputFrame] += mono * panLeft * dryGain * frontGain;
				if (outputChannels > 1 && outputs[1])
					outputs[1][outputFrame] += mono * panRight * dryGain * frontGain;
				if (outputChannels == 4)
				{
					if (outputs[2])
						outputs[2][outputFrame] += mono * panLeft * dryGain * rearGain;
					if (outputs[3])
						outputs[3][outputFrame] += mono * panRight * dryGain * rearGain;
				}
				else if (outputChannels >= 6)
				{
					if (outputs[4])
						outputs[4][outputFrame] += mono * panLeft * dryGain * rearGain;
					if (outputs[5])
						outputs[5][outputFrame] += mono * panRight * dryGain * rearGain;
					if (outputChannels >= 8)
					{
						if (outputs[6])
							outputs[6][outputFrame] += mono * panLeft * dryGain * 0.5f;
						if (outputs[7])
							outputs[7][outputFrame] += mono * panRight * dryGain * 0.5f;
					}
				}
				F32 const wetGain = spatialVolume * sampleWetLevel * masterWetLevel;
				wetLeft[outputFrame] += mono * panLeft * wetGain;
				wetRight[outputFrame] += mono * panRight * wetGain;
			}
			else
			{
				F32 const dryGain = sampleDryLevel * masterDryLevel * masterVolume;
				if (outputChannels > 0 && outputs[0])
					outputs[0][outputFrame] += left * sampleLeftVolume * dryGain;
				if (outputChannels > 1 && outputs[1])
					outputs[1][outputFrame] += rightSample * sampleRightVolume * dryGain;
				F32 const wetGain = sampleWetLevel * masterWetLevel * masterVolume;
				wetLeft[outputFrame] += left * sampleLeftVolume * wetGain;
				wetRight[outputFrame] += rightSample * sampleRightVolume * wetGain;
			}

			sample.cursorFrame += step;
			if (sample.loopActive && sample.cursorFrame >= static_cast<double>(loopEnd))
			{
				if (sample.loopCount == 0 || sample.remainingLoops > 1)
				{
					if (sample.remainingLoops > 1)
						--sample.remainingLoops;
					double const overflow = sample.cursorFrame - static_cast<double>(loopEnd);
					sample.cursorFrame = static_cast<double>(loopStart) + std::fmod(overflow, static_cast<double>(loopEnd - loopStart));
				}
				else
				{
					sample.loopActive = false;
				}
			}
		}
	}

	void DriverState::audioDeviceIOCallbackWithContext(
		float const *const *,
		int,
		float *const *outputChannelData,
		int numOutputChannels,
		int numSamples,
		juce::AudioIODeviceCallbackContext const &)
	{
		for (int channel = 0; channel < numOutputChannels; ++channel)
		{
			if (outputChannelData[channel])
				juce::FloatVectorOperations::clear(outputChannelData[channel], numSamples);
		}

		s_callbackBlocks.fetch_add(1, std::memory_order_relaxed);
		std::unique_lock<std::recursive_mutex> lock(s_mutex, std::try_to_lock);
		if (!lock.owns_lock())
		{
			s_callbackLockMisses.fetch_add(1, std::memory_order_relaxed);
			// Masking: replay the previous block instead of leaving silence --
			// stale data, never no data (Miles semantics). The cursor did not
			// advance, so the next mixed block continues where this one would
			// have started; the replay is ~one block of doubled audio, not a
			// gap. Consecutive misses fade so a long lock hold decays to
			// silence instead of buzzing at the block period.
			int const replayFrames = (std::min)(numSamples, lastBlockSamples);
			for (int channel = 0; channel < numOutputChannels && channel < lastBlock.getNumChannels(); ++channel)
			{
				if (outputChannelData[channel] && replayFrames > 0)
					juce::FloatVectorOperations::copyWithMultiply(outputChannelData[channel], lastBlock.getReadPointer(channel), lastBlockRepeatGain, replayFrames);
			}
			lastBlockRepeatGain *= 0.6f;
			return;
		}
		lastBlockRepeatGain = 1.0f;
		if (numOutputChannels <= 0 || numSamples <= 0)
			return;

		if (wetBuffer.getNumSamples() < numSamples)
			wetBuffer.setSize(2, numSamples, false, false, true);
		wetBuffer.clear(0, 0, numSamples);
		wetBuffer.clear(1, 0, numSamples);
		float *wetLeft = wetBuffer.getWritePointer(0);
		float *wetRight = wetBuffer.getWritePointer(1);

		outputChannels = numOutputChannels;
		for (auto &entry : s_samples)
		{
			if (entry.second->driver == handle)
				mixSample(*this, *entry.second, outputChannelData, numOutputChannels, numSamples, wetLeft, wetRight);
		}

		reverb.processStereo(wetLeft, wetRight, numSamples);
		if (outputChannelData[0])
			juce::FloatVectorOperations::add(outputChannelData[0], wetLeft, numSamples);
		if (numOutputChannels > 1 && outputChannelData[1])
			juce::FloatVectorOperations::add(outputChannelData[1], wetRight, numSamples);

		//-- snapshot the finished block for lock-miss masking (grows once, then reuses)
		lastBlock.setSize(numOutputChannels, numSamples, false, false, true);
		for (int channel = 0; channel < numOutputChannels; ++channel)
		{
			if (outputChannelData[channel])
				lastBlock.copyFrom(channel, 0, outputChannelData[channel], numSamples);
			else
				lastBlock.clear(channel, 0, numSamples);
		}
		lastBlockSamples = numSamples;
	}

	void DriverState::audioDeviceAboutToStart(juce::AudioIODevice *device)
	{
		std::lock_guard<std::recursive_mutex> lock(s_mutex);
		if (device)
		{
			sampleRate = device->getCurrentSampleRate();
			bufferSize = device->getCurrentBufferSizeSamples();
			outputChannels = device->getActiveOutputChannels().countNumberOfSetBits();
			wetBuffer.setSize(2, (std::max)(bufferSize, 64), false, false, true);
			reverb.setSampleRate(sampleRate);
			applyRoomPreset(*this);
		}
		deviceRunning.store(true, std::memory_order_release);
	}

	void DriverState::audioDeviceStopped()
	{
		deviceRunning.store(false, std::memory_order_release);
	}

	void DriverState::audioDeviceError(juce::String const &message)
	{
		std::lock_guard<std::recursive_mutex> lock(s_mutex);
		setError(message.toRawUTF8());
	}

	void applyMix(SampleState &)
	{
		// Mix state is read directly by the JUCE real-time callback.
	}

	bool createVoice(SampleState &sample)
	{
		if (!findDriver(sample.driver) || sample.pcm.getNumSamples() == 0)
		{
			setError("JUCE audio driver or decoded sample is unavailable");
			return false;
		}
		return true;
	}

	void destroyVoice(SampleState &sample)
	{
		sample.status.store(SMP_DONE, std::memory_order_relaxed);
		sample.completed.store(false, std::memory_order_release);
	}

	FrameIndex getCurrentFrame(SampleState const &sample)
	{
		return (std::min)(sample.totalFrames, static_cast<FrameIndex>((std::max)(sample.cursorFrame, 0.0)));
	}

	bool startSample(SampleState &sample)
	{
		if (sample.pcm.getNumSamples() == 0 || sample.totalFrames == 0 || !findDriver(sample.driver))
		{
			setError("Cannot start a sample before audio data and a JUCE device are assigned");
			return false;
		}
		sample.seekFrame = (std::min)(sample.seekFrame, sample.totalFrames - 1);
		sample.cursorFrame = static_cast<double>(sample.seekFrame);
		sample.remainingLoops = sample.loopCount;
		FrameIndex const loopEnd = sample.loopEndOffset < 0 ? sample.totalFrames : mapEncodedOffsetToFrame(sample, sample.loopEndOffset);
		sample.loopActive = (sample.loopCount == 0 || sample.loopCount > 1) && loopEnd > sample.seekFrame;
		sample.filterLeft = 0.0f;
		sample.filterRight = 0.0f;
		sample.completed.store(false, std::memory_order_release);
		sample.status.store(SMP_PLAYING, std::memory_order_relaxed);
		return true;
	}

	void releaseSample(HSAMPLE sampleHandle)
	{
		auto const iter = s_samples.find(sampleHandle);
		if (iter == s_samples.end())
			return;
		// stop any in-flight background fill from touching this (soon-freed) sample
		iter->second->fillToken = 0;
		destroyVoice(*iter->second);
		delete sampleHandle;
		s_samples.erase(iter);
	}

	void closeStream(HSTREAM streamHandle)
	{
		auto const iter = s_streams.find(streamHandle);
		if (iter == s_streams.end())
			return;
		HSAMPLE const sample = iter->second->sample;
		delete streamHandle;
		s_streams.erase(iter);
		releaseSample(sample);
	}

	void destroyDriver(HDIGDRIVER driverHandle)
	{
		auto const iter = s_drivers.find(driverHandle);
		if (iter == s_drivers.end())
			return;
		DriverState &driver = *iter->second;
		driver.deviceManager.removeAudioCallback(&driver);
		driver.deviceManager.closeAudioDevice();
		delete driverHandle;
		s_drivers.erase(iter);
	}

	bool readStreamFile(char const *filename, std::vector<U8> &data)
	{
		if (!s_fileOpenCallback || !s_fileCloseCallback || !s_fileSeekCallback || !s_fileReadCallback)
		{
			setError("SWG audio file callbacks are not installed");
			return false;
		}

		UINTa fileHandle = 0;
		if (!s_fileOpenCallback(filename, &fileHandle))
		{
			s_fileError = AIL_FILE_NOT_FOUND;
			setError("Unable to open streamed audio from the TreeFile system");
			return false;
		}
		S32 const fileLength = s_fileSeekCallback(fileHandle, 0, AIL_FILE_SEEK_END);
		s_fileSeekCallback(fileHandle, 0, AIL_FILE_SEEK_BEGIN);
		if (fileLength <= 0)
		{
			s_fileCloseCallback(fileHandle);
			s_fileError = AIL_CANT_READ_FILE;
			setError("Streamed audio file is empty");
			return false;
		}

		data.resize(static_cast<size_t>(fileLength));
		U32 totalRead = 0;
		while (totalRead < static_cast<U32>(fileLength))
		{
			U32 const bytesRead = s_fileReadCallback(fileHandle, data.data() + totalRead, static_cast<U32>(fileLength) - totalRead);
			if (bytesRead == 0)
				break;
			totalRead += bytesRead;
		}
		s_fileCloseCallback(fileHandle);
		if (totalRead != static_cast<U32>(fileLength))
		{
			data.clear();
			s_fileError = AIL_CANT_READ_FILE;
			setError("Unable to read the complete streamed audio file");
			return false;
		}
		s_fileError = AIL_NO_ERROR;
		return true;
	}
} // namespace JuceMilesNamespace

using namespace JuceMilesNamespace;

extern "C"
{
	S32 AILCALL AIL_startup(void)
	{
		std::lock_guard<std::recursive_mutex> lock(s_mutex);
		if (!s_started)
		{
			s_juceInitialiser = std::make_unique<juce::ScopedJuceInitialiser_GUI>();
			s_formatManager.registerBasicFormats();
			s_preferences[DIG_MIXER_CHANNELS] = 64;
			s_started = true;
		}
		s_lastError.clear();
		return 1;
	}

	void AILCALL AIL_shutdown(void)
	{
		std::lock_guard<std::recursive_mutex> lock(s_mutex);
		while (!s_streams.empty())
			closeStream(s_streams.begin()->first);
		while (!s_samples.empty())
			releaseSample(s_samples.begin()->first);
		while (!s_drivers.empty())
			destroyDriver(s_drivers.begin()->first);
		s_formatManager.clearFormats();
		s_juceInitialiser.reset();
		s_started = false;
	}

	SINTa AILCALL AIL_get_preference(U32 number)
	{
		std::lock_guard<std::recursive_mutex> lock(s_mutex);
		auto const iter = s_preferences.find(number);
		return iter == s_preferences.end() ? 0 : iter->second;
	}

	SINTa AILCALL AIL_set_preference(U32 number, SINTa value)
	{
		std::lock_guard<std::recursive_mutex> lock(s_mutex);
		SINTa const previous = AIL_get_preference(number);
		s_preferences[number] = value;
		return previous;
	}

	char *AILCALL AIL_last_error(void)
	{
		return s_lastError.empty() ? const_cast<char *>("") : const_cast<char *>(s_lastError.c_str());
	}

	void AILCALL AIL_lock(void)
	{
		s_mutex.lock();
	}
	void AILCALL AIL_unlock(void)
	{
		s_mutex.unlock();
	}

	char *AILCALL AIL_set_redist_directory(char const *directory)
	{
		std::lock_guard<std::recursive_mutex> lock(s_mutex);
		s_redistDirectory = directory ? directory : "";
		return const_cast<char *>(s_redistDirectory.c_str());
	}

	HDIGDRIVER AILCALL AIL_open_digital_driver(U32 frequency, S32, S32 channel, U32)
	{
		std::lock_guard<std::recursive_mutex> lock(s_mutex);
		if (!s_started)
			AIL_startup();

		auto state = std::make_unique<DriverState>();
		state->channelSpec = static_cast<MSS_MC_SPEC>(channel);
		juce::AudioDeviceManager::AudioDeviceSetup preferredSetup;
		preferredSetup.sampleRate = frequency ? static_cast<double>(frequency) : 0.0;
		juce::String error = state->deviceManager.initialise(
			0,
			requestedChannelCount(channel),
			nullptr,
			true,
			juce::String(),
			&preferredSetup);
		if (error.isNotEmpty())
		{
			error = state->deviceManager.initialiseWithDefaultDevices(0, 2);
			state->channelSpec = MSS_MC_STEREO;
		}
		if (error.isNotEmpty() || !state->deviceManager.getCurrentAudioDevice())
		{
			setError(error.isNotEmpty() ? error.toRawUTF8() : "JUCE could not open the default output device");
			return nullptr;
		}

		HDIGDRIVER token = new _DIG_DRIVER;
		std::memset(token, 0, sizeof(*token));
		state->handle = token;
		auto inserted = s_drivers.emplace(token, std::move(state));
		DriverState &driver = *inserted.first->second;
		driver.deviceManager.addAudioCallback(&driver);
		s_lastError.clear();
		return token;
	}

	void AILCALL AIL_close_digital_driver(HDIGDRIVER driver)
	{
		std::lock_guard<std::recursive_mutex> lock(s_mutex);
		destroyDriver(driver);
	}

	S32 AILCALL AIL_digital_CPU_percent(HDIGDRIVER driverHandle)
	{
		std::lock_guard<std::recursive_mutex> lock(s_mutex);
		DriverState *driver = findDriver(driverHandle);
		return driver ? static_cast<S32>(driver->deviceManager.getCpuUsage() * 100.0) : 0;
	}

	S32 AILCALL AIL_digital_latency(HDIGDRIVER driverHandle)
	{
		std::lock_guard<std::recursive_mutex> lock(s_mutex);
		DriverState *driver = findDriver(driverHandle);
		return driver && driver->sampleRate > 0.0 ? static_cast<S32>(driver->bufferSize * 1000.0 / driver->sampleRate) : 0;
	}

	void AILCALL AIL_digital_configuration(HDIGDRIVER driverHandle, S32 *rate, S32 *format, char *description)
	{
		std::lock_guard<std::recursive_mutex> lock(s_mutex);
		DriverState *driver = findDriver(driverHandle);
		if (rate)
			*rate = driver ? static_cast<S32>(driver->sampleRate) : 0;
		if (format)
			*format = driver ? DIG_F_16BITS_MASK | (driver->outputChannels > 1 ? DIG_F_STEREO_MASK : 0) : 0;
		if (description)
			std::strcpy(description, driver ? "JUCE 8.0.14 / Windows Audio" : "JUCE unavailable");
	}

	void AILCALL AIL_set_digital_master_volume_level(HDIGDRIVER driverHandle, F32 volume)
	{
		// per-frame hot setter -- lock-free by design (see s_mutex comment)
		DriverState *driver = findDriver(driverHandle);
		if (driver)
			driver->masterVolume.store(clampValue(volume, 0.0f, 1.0f), std::memory_order_relaxed);
	}

	F32 AILCALL AIL_digital_master_volume_level(HDIGDRIVER driverHandle)
	{
		// per-frame hot poll -- lock-free by design (see s_mutex comment)
		DriverState *driver = findDriver(driverHandle);
		return driver ? driver->masterVolume.load(std::memory_order_relaxed) : 0.0f;
	}

	void AILCALL AIL_set_digital_master_reverb_levels(HDIGDRIVER driverHandle, F32 dry, F32 wet)
	{
		// per-frame hot setter -- lock-free by design (see s_mutex comment)
		DriverState *driver = findDriver(driverHandle);
		if (driver)
		{
			driver->masterDryLevel.store(clampValue(dry, 0.0f, 1.0f), std::memory_order_relaxed);
			driver->masterWetLevel.store(clampValue(wet, 0.0f, 1.0f), std::memory_order_relaxed);
		}
	}

	void AILCALL AIL_digital_master_reverb_levels(HDIGDRIVER driverHandle, F32 *dry, F32 *wet)
	{
		// per-frame hot poll -- lock-free by design (see s_mutex comment)
		DriverState *driver = findDriver(driverHandle);
		if (dry)
			*dry = driver ? driver->masterDryLevel.load(std::memory_order_relaxed) : 0.0f;
		if (wet)
			*wet = driver ? driver->masterWetLevel.load(std::memory_order_relaxed) : 0.0f;
	}

	HSAMPLE AILCALL AIL_allocate_sample_handle(HDIGDRIVER driver)
	{
		std::lock_guard<std::recursive_mutex> lock(s_mutex);
		if (!findDriver(driver))
			return nullptr;
		HSAMPLE token = new _SAMPLE;
		std::memset(token, 0, sizeof(*token));
		s_samples.emplace(token, std::make_unique<SampleState>(driver));
		return token;
	}

	void AILCALL AIL_release_sample_handle(HSAMPLE sample)
	{
		std::lock_guard<std::recursive_mutex> lock(s_mutex);
		releaseSample(sample);
	}

	S32 AILCALL AIL_set_named_sample_file(HSAMPLE sampleHandle, C8 const *, void const *fileImage, U32 fileSize, S32)
	{
		if (!fileImage || fileSize == 0)
		{
			setError("Cannot assign an empty audio file");
			return 0;
		}

		//-- decode OUTSIDE the lock. The device callback takes s_mutex with try_to_lock
		//   and plays SILENCE when it loses; holding the lock across a decode made every
		//   sample load an audible gap -- measured as ~60 missed callback blocks (~18%)
		//   during the login screen's ~98 sample loads. decodeEncoded touches no shared
		//   state, so only the copy-in needs the lock, same as the streamed fill path.
		std::vector<U8> encoded(static_cast<U8 const *>(fileImage), static_cast<U8 const *>(fileImage) + fileSize);
		DecodedAudio decoded;
		bool const ok = decodeEncoded(encoded, decoded);

		std::lock_guard<std::recursive_mutex> lock(s_mutex);
		SampleState *sample = findSample(sampleHandle);
		if (!sample)
		{
			setError("Cannot assign audio to a released sample");
			return 0;
		}
		destroyVoice(*sample);
		sample->fillToken = 0;   // supersede any background fill still in flight
		sample->encodedData = std::move(encoded);
		if (!ok)
			return 0;
		applyDecoded(*sample, decoded);
		return createVoice(*sample) ? 1 : 0;
	}

	S32 AILCALL AIL_set_sample_file(HSAMPLE sample, void const *fileImage, S32 block)
	{
		if (!fileImage)
			return 0;
		U8 const *data = static_cast<U8 const *>(fileImage);
		if (!isWave(data, 12))
		{
			setError("AIL_set_sample_file requires RIFF/WAVE data in the SWG JUCE backend");
			return 0;
		}
		return AIL_set_named_sample_file(sample, ".wav", fileImage, readU32(data + 4) + 8, block);
	}

	void AILCALL AIL_start_sample(HSAMPLE sampleHandle)
	{
		std::lock_guard<std::recursive_mutex> lock(s_mutex);
		SampleState *sample = findSample(sampleHandle);
		if (sample)
			startSample(*sample);
	}

	void AILCALL AIL_stop_sample(HSAMPLE sampleHandle)
	{
		std::lock_guard<std::recursive_mutex> lock(s_mutex);
		SampleState *sample = findSample(sampleHandle);
		if (sample && sample->status.load(std::memory_order_relaxed) == SMP_PLAYING)
			sample->status.store(SMP_STOPPED, std::memory_order_relaxed);
	}

	void AILCALL AIL_end_sample(HSAMPLE sampleHandle)
	{
		std::lock_guard<std::recursive_mutex> lock(s_mutex);
		SampleState *sample = findSample(sampleHandle);
		if (sample)
		{
			sample->status.store(SMP_DONE, std::memory_order_relaxed);
			sample->completed.store(false, std::memory_order_release);
		}
	}

	U32 AILCALL AIL_sample_status(HSAMPLE sampleHandle)
	{
		// per-frame hot poll -- lock-free by design (see s_mutex comment)
		SampleState *sample = findSample(sampleHandle);
		return sample ? sample->status.load(std::memory_order_relaxed) : SMP_FREE;
	}

	S32 AILCALL AIL_active_sample_count(HDIGDRIVER driver)
	{
		// per-frame hot poll -- lock-free by design (see s_mutex comment)
		S32 count = 0;
		for (auto const &entry : s_samples)
			if (entry.second->driver == driver && entry.second->status.load(std::memory_order_relaxed) == SMP_PLAYING)
				++count;
		return count;
	}

	void AILCALL AIL_set_sample_playback_rate(HSAMPLE sampleHandle, S32 playbackRate)
	{
		// per-frame hot setter -- lock-free by design (see s_mutex comment)
		SampleState *sample = findSample(sampleHandle);
		if (sample)
			sample->playbackRate.store(playbackRate, std::memory_order_relaxed);
	}

	S32 AILCALL AIL_sample_playback_rate(HSAMPLE sampleHandle)
	{
		// per-frame hot poll -- lock-free by design (see s_mutex comment)
		SampleState *sample = findSample(sampleHandle);
		return sample ? sample->playbackRate.load(std::memory_order_relaxed) : 0;
	}

	void AILCALL AIL_set_sample_volume_levels(HSAMPLE sampleHandle, F32 left, F32 right)
	{
		// per-frame hot setter -- lock-free by design (see s_mutex comment)
		SampleState *sample = findSample(sampleHandle);
		if (sample)
		{
			sample->leftVolume.store(left, std::memory_order_relaxed);
			sample->rightVolume.store(right, std::memory_order_relaxed);
		}
	}

	void AILCALL AIL_sample_volume_levels(HSAMPLE sampleHandle, F32 *left, F32 *right)
	{
		// per-frame hot poll -- lock-free by design (see s_mutex comment)
		SampleState *sample = findSample(sampleHandle);
		if (left)
			*left = sample ? sample->leftVolume.load(std::memory_order_relaxed) : 0.0f;
		if (right)
			*right = sample ? sample->rightVolume.load(std::memory_order_relaxed) : 0.0f;
	}

	void AILCALL AIL_set_sample_reverb_levels(HSAMPLE sampleHandle, F32 dry, F32 wet)
	{
		// per-frame hot setter -- lock-free by design (see s_mutex comment)
		SampleState *sample = findSample(sampleHandle);
		if (sample)
		{
			sample->dryLevel.store(clampValue(dry, 0.0f, 1.0f), std::memory_order_relaxed);
			sample->wetLevel.store(clampValue(wet, 0.0f, 1.0f), std::memory_order_relaxed);
		}
	}

	void AILCALL AIL_sample_reverb_levels(HSAMPLE sampleHandle, F32 *dry, F32 *wet)
	{
		// per-frame hot poll -- lock-free by design (see s_mutex comment)
		SampleState *sample = findSample(sampleHandle);
		if (dry)
			*dry = sample ? sample->dryLevel.load(std::memory_order_relaxed) : 0.0f;
		if (wet)
			*wet = sample ? sample->wetLevel.load(std::memory_order_relaxed) : 0.0f;
	}

	void AILCALL AIL_set_sample_loop_count(HSAMPLE sampleHandle, S32 count)
	{
		std::lock_guard<std::recursive_mutex> lock(s_mutex);
		SampleState *sample = findSample(sampleHandle);
		if (sample)
			sample->loopCount = count;
	}

	S32 AILCALL AIL_sample_loop_count(HSAMPLE sampleHandle)
	{
		std::lock_guard<std::recursive_mutex> lock(s_mutex);
		SampleState *sample = findSample(sampleHandle);
		return sample ? sample->loopCount : 0;
	}

	void AILCALL AIL_set_sample_loop_block(HSAMPLE sampleHandle, S32 startOffset, S32 endOffset)
	{
		std::lock_guard<std::recursive_mutex> lock(s_mutex);
		SampleState *sample = findSample(sampleHandle);
		if (sample)
		{
			sample->loopStartOffset = startOffset;
			sample->loopEndOffset = endOffset;
		}
	}

	AILSAMPLECB AILCALL AIL_register_EOS_callback(HSAMPLE sampleHandle, AILSAMPLECB callback)
	{
		std::lock_guard<std::recursive_mutex> lock(s_mutex);
		SampleState *sample = findSample(sampleHandle);
		if (!sample)
			return nullptr;
		AILSAMPLECB const previous = sample->eosCallback;
		sample->eosCallback = callback;
		return previous;
	}

	void AILCALL AIL_set_sample_ms_position(HSAMPLE sampleHandle, S32 milliseconds)
	{
		std::lock_guard<std::recursive_mutex> lock(s_mutex);
		SampleState *sample = findSample(sampleHandle);
		if (!sample || sample->sourceSampleRate <= 0)
			return;
		sample->seekFrame = clampValue<FrameIndex>(
			static_cast<FrameIndex>((std::max)(milliseconds, 0)) * sample->sourceSampleRate / 1000,
			0,
			sample->totalFrames ? sample->totalFrames - 1 : 0);
		if (sample->status.load(std::memory_order_relaxed) == SMP_PLAYING)
			startSample(*sample);
		else
			sample->cursorFrame = static_cast<double>(sample->seekFrame);
	}

	void AILCALL AIL_sample_ms_position(HSAMPLE sampleHandle, S32 *totalMilliseconds, S32 *currentMilliseconds)
	{
		std::lock_guard<std::recursive_mutex> lock(s_mutex);
		SampleState *sample = findSample(sampleHandle);
		if (totalMilliseconds)
			*totalMilliseconds = sample && sample->sourceSampleRate ? static_cast<S32>(sample->totalFrames * 1000 / sample->sourceSampleRate) : 0;
		if (currentMilliseconds)
			*currentMilliseconds = sample && sample->sourceSampleRate ? static_cast<S32>(getCurrentFrame(*sample) * 1000 / sample->sourceSampleRate) : 0;
	}

	void AILCALL AIL_set_sample_position(HSAMPLE sampleHandle, U32 position)
	{
		std::lock_guard<std::recursive_mutex> lock(s_mutex);
		SampleState *sample = findSample(sampleHandle);
		if (!sample)
			return;
		sample->seekFrame = mapEncodedOffsetToFrame(*sample, static_cast<S32>(position));
		if (sample->status.load(std::memory_order_relaxed) == SMP_PLAYING)
			startSample(*sample);
		else
			sample->cursorFrame = static_cast<double>(sample->seekFrame);
	}

	U32 AILCALL AIL_sample_position(HSAMPLE sampleHandle)
	{
		std::lock_guard<std::recursive_mutex> lock(s_mutex);
		SampleState *sample = findSample(sampleHandle);
		return sample ? mapFrameToEncodedOffset(*sample, getCurrentFrame(*sample)) : 0;
	}

	void AILCALL AIL_set_sample_3D_position(HSAMPLE sampleHandle, F32 x, F32 y, F32 z)
	{
		// per-frame hot setter -- lock-free by design (see s_mutex comment)
		SampleState *sample = findSample(sampleHandle);
		if (sample)
		{
			sample->spatialized.store(true, std::memory_order_relaxed);
			sample->position.set(x, y, z);
		}
	}

	void AILCALL AIL_set_sample_3D_velocity_vector(HSAMPLE sampleHandle, F32 x, F32 y, F32 z)
	{
		// per-frame hot setter -- lock-free by design (see s_mutex comment)
		SampleState *sample = findSample(sampleHandle);
		if (sample)
			sample->velocity.set(x, y, z);
	}

	void AILCALL AIL_set_sample_3D_distances(HSAMPLE sampleHandle, F32 maxDistance, F32 minDistance, S32)
	{
		// per-frame hot setter -- lock-free by design (see s_mutex comment)
		SampleState *sample = findSample(sampleHandle);
		if (sample)
		{
			F32 const boundedMax = (std::max)(maxDistance, 1.0f);
			sample->maxDistance.store(boundedMax, std::memory_order_relaxed);
			sample->minDistance.store(clampValue(minDistance, 0.0f, boundedMax), std::memory_order_relaxed);
			sample->spatialized.store(true, std::memory_order_relaxed);
		}
	}

	void AILCALL AIL_set_sample_obstruction(HSAMPLE sampleHandle, F32 value)
	{
		// per-frame hot setter -- lock-free by design (see s_mutex comment)
		SampleState *sample = findSample(sampleHandle);
		if (sample)
			sample->obstruction.store(clampValue(value, 0.0f, 1.0f), std::memory_order_relaxed);
	}

	void AILCALL AIL_set_sample_occlusion(HSAMPLE sampleHandle, F32 value)
	{
		// per-frame hot setter -- lock-free by design (see s_mutex comment)
		SampleState *sample = findSample(sampleHandle);
		if (sample)
			sample->occlusion.store(clampValue(value, 0.0f, 1.0f), std::memory_order_relaxed);
	}

	HSTREAM AILCALL AIL_open_stream(HDIGDRIVER driver, char const *filename, S32)
	{
		// No function-wide lock: the TreeFile read, header parse, and inline
		// first-second decode below were the audio lock's single largest remaining
		// hold (0.1-2.2ms plus file I/O at every music/ambience change -- each
		// open a near-certain dropout block). Safe unlocked on the game thread:
		// the fresh sample stays SMP_DONE until AIL_start_stream, and mixSample's
		// status check returns before it touches pcm or metadata. The map
		// mutations (allocate/release/stream insert) still lock internally.
		std::vector<U8> fileData;
		if (!findDriver(driver) || !filename || !readStreamFile(filename, fileData))
			return nullptr;
		HSAMPLE const sampleHandle = AIL_allocate_sample_handle(driver);
		if (!sampleHandle)
			return nullptr;

		// Header-first open (metadata correct immediately, PCM filled in the background --
		// see openStreamedSample). If the header parse fails, fall back to the original
		// fully synchronous decode so behaviour is never worse than before.
		SampleState *const sample = findSample(sampleHandle);
		bool opened = false;
		if (sample)
		{
			destroyVoice(*sample);
			sample->encodedData = fileData;   // keep fileData for the sync fallback
			opened = openStreamedSample(*sample, sampleHandle, filename);
		}
		if (!opened)
		{
			char const *extension = std::strrchr(filename, '.');
			if (!AIL_set_named_sample_file(sampleHandle, extension ? extension : "", fileData.data(), static_cast<U32>(fileData.size()), 0))
			{
				AIL_release_sample_handle(sampleHandle);
				return nullptr;
			}
		}
		HSTREAM token = new _STREAM;
		std::memset(token, 0, sizeof(*token));
		auto stream = std::make_unique<StreamState>();
		stream->sample = sampleHandle;
		stream->filename = filename;
		{
			std::lock_guard<std::recursive_mutex> lock(s_mutex);
			s_streams.emplace(token, std::move(stream));
			findSample(sampleHandle)->ownerStream = token;
		}
		return token;
	}

	void AILCALL AIL_close_stream(HSTREAM stream)
	{
		std::lock_guard<std::recursive_mutex> lock(s_mutex);
		closeStream(stream);
	}

	HSAMPLE AILCALL AIL_stream_sample_handle(HSTREAM streamHandle)
	{
		std::lock_guard<std::recursive_mutex> lock(s_mutex);
		StreamState *stream = findStream(streamHandle);
		return stream ? stream->sample : nullptr;
	}

	void AILCALL AIL_start_stream(HSTREAM stream)
	{
		HSAMPLE const sample = AIL_stream_sample_handle(stream);
		if (sample)
			AIL_start_sample(sample);
	}

	void AILCALL AIL_pause_stream(HSTREAM stream, S32 onoff)
	{
		std::lock_guard<std::recursive_mutex> lock(s_mutex);
		SampleState *sample = findSample(AIL_stream_sample_handle(stream));
		if (!sample)
			return;
		if (onoff)
			sample->status.store(SMP_STOPPED, std::memory_order_relaxed);
		else if (sample->status.load(std::memory_order_relaxed) == SMP_STOPPED)
			sample->status.store(SMP_PLAYING, std::memory_order_relaxed);
	}

	void AILCALL AIL_set_stream_loop_count(HSTREAM stream, S32 count)
	{
		AIL_set_sample_loop_count(AIL_stream_sample_handle(stream), count);
	}
	void AILCALL AIL_set_stream_loop_block(HSTREAM stream, S32 start, S32 end)
	{
		AIL_set_sample_loop_block(AIL_stream_sample_handle(stream), start, end);
	}
	S32 AILCALL AIL_stream_status(HSTREAM stream)
	{
		return static_cast<S32>(AIL_sample_status(AIL_stream_sample_handle(stream)));
	}

	AILSTREAMCB AILCALL AIL_register_stream_callback(HSTREAM streamHandle, AILSTREAMCB callback)
	{
		std::lock_guard<std::recursive_mutex> lock(s_mutex);
		StreamState *stream = findStream(streamHandle);
		if (!stream)
			return nullptr;
		AILSTREAMCB const previous = stream->callback;
		stream->callback = callback;
		return previous;
	}

	void AILCALL AIL_set_stream_ms_position(HSTREAM stream, S32 milliseconds)
	{
		AIL_set_sample_ms_position(AIL_stream_sample_handle(stream), milliseconds);
	}
	void AILCALL AIL_stream_ms_position(HSTREAM stream, S32 *total, S32 *current)
	{
		AIL_sample_ms_position(AIL_stream_sample_handle(stream), total, current);
	}

	void AILCALL AIL_set_file_callbacks(AIL_file_open_callback openCallback, AIL_file_close_callback closeCallback, AIL_file_seek_callback seekCallback, AIL_file_read_callback readCallback)
	{
		std::lock_guard<std::recursive_mutex> lock(s_mutex);
		s_fileOpenCallback = openCallback;
		s_fileCloseCallback = closeCallback;
		s_fileSeekCallback = seekCallback;
		s_fileReadCallback = readCallback;
	}

	S32 AILCALL AIL_file_error(void)
	{
		return s_fileError;
	}

	S32 AILCALL AIL_file_type(void const *fileImage, U32 fileSize)
	{
		if (!fileImage)
			return AILFILETYPE_UNKNOWN;
		U8 const *data = static_cast<U8 const *>(fileImage);
		if (isWave(data, fileSize))
			return AILFILETYPE_PCM_WAV;
		if (isOgg(data, fileSize))
			return AILFILETYPE_OGG_VORBIS;
		if (isMp3(data, fileSize))
			return AILFILETYPE_MPEG_L3_AUDIO;
		return AILFILETYPE_UNKNOWN;
	}

	S32 AILCALL AIL_WAV_info(void const *fileImage, AILSOUNDINFO *info)
	{
		if (!fileImage || !info)
			return 0;
		U8 const *data = static_cast<U8 const *>(fileImage);
		U32 const fileSize = isWave(data, 12) ? readU32(data + 4) + 8 : 0;
		if (!fileSize)
			return 0;
		size_t formatOffset = 0;
		size_t dataOffset = 0;
		U32 formatLength = 0;
		U32 dataLength = 0;
		if (!findWaveChunk(data, fileSize, "fmt ", formatOffset, formatLength) || formatLength < 16 || !findWaveChunk(data, fileSize, "data", dataOffset, dataLength))
			return 0;
		std::memset(info, 0, sizeof(*info));
		U16 const waveFormat = readU16(data + formatOffset);
		info->channels = readU16(data + formatOffset + 2);
		info->rate = readU32(data + formatOffset + 4);
		info->block_size = readU16(data + formatOffset + 12);
		info->bits = readU16(data + formatOffset + 14);
		info->data_ptr = data + dataOffset;
		info->data_len = dataLength;
		info->samples = info->block_size ? dataLength / info->block_size : 0;
		info->initial_ptr = fileImage;
		info->format = (info->channels > 1 ? DIG_F_STEREO_MASK : 0) | (info->bits == 16 ? DIG_F_16BITS_MASK : 0);
		if (waveFormat != WAVE_FORMAT_PCM && waveFormat != 3u)
			info->format |= DIG_F_ADPCM_MASK;
		return 1;
	}

	void AILCALL AIL_set_3D_rolloff_factor(HDIGDRIVER driverHandle, F32 factor)
	{
		// per-frame hot setter -- lock-free by design (see s_mutex comment)
		DriverState *driver = findDriver(driverHandle);
		if (driver)
			driver->rolloffFactor.store(factor, std::memory_order_relaxed);
	}

	void AILCALL AIL_set_listener_3D_position(HDIGDRIVER driverHandle, F32 x, F32 y, F32 z)
	{
		// per-frame hot setter -- lock-free by design (see s_mutex comment)
		DriverState *driver = findDriver(driverHandle);
		if (driver)
			driver->listenerPosition.set(x, y, z);
	}

	void AILCALL AIL_set_listener_3D_velocity_vector(HDIGDRIVER driverHandle, F32 x, F32 y, F32 z)
	{
		// per-frame hot setter -- lock-free by design (see s_mutex comment)
		DriverState *driver = findDriver(driverHandle);
		if (driver)
			driver->listenerVelocity.set(x, y, z);
	}

	void AILCALL AIL_set_listener_3D_orientation(HDIGDRIVER driverHandle, F32 faceX, F32 faceY, F32 faceZ, F32 upX, F32 upY, F32 upZ)
	{
		// per-frame hot setter -- lock-free by design (see s_mutex comment)
		DriverState *driver = findDriver(driverHandle);
		if (driver)
		{
			Vector3 const front = normalize({faceX, faceY, faceZ}, {0.0f, 0.0f, -1.0f});
			Vector3 const up = normalize({upX, upY, upZ}, {0.0f, 1.0f, 0.0f});
			driver->listenerFront.set(front.x, front.y, front.z);
			driver->listenerUp.set(up.x, up.y, up.z);
		}
	}

	void AILCALL AIL_set_room_type(HDIGDRIVER driverHandle, S32 roomType)
	{
		std::lock_guard<std::recursive_mutex> lock(s_mutex);
		DriverState *driver = findDriver(driverHandle);
		if (driver)
		{
			driver->roomType = roomType;
			applyRoomPreset(*driver);
		}
	}

	S32 AILCALL AIL_room_type(HDIGDRIVER driverHandle)
	{
		std::lock_guard<std::recursive_mutex> lock(s_mutex);
		DriverState *driver = findDriver(driverHandle);
		return driver ? driver->roomType : ENVIRONMENT_GENERIC;
	}

	MSSVECTOR3D *AILCALL AIL_speaker_configuration(HDIGDRIVER driverHandle, S32 *physicalChannels, S32 *logicalChannels, F32 *falloffPower, MSS_MC_SPEC *channelSpec)
	{
		std::lock_guard<std::recursive_mutex> lock(s_mutex);
		DriverState *driver = findDriver(driverHandle);
		if (physicalChannels)
			*physicalChannels = driver ? driver->outputChannels : 0;
		if (logicalChannels)
			*logicalChannels = driver ? driver->outputChannels : 0;
		if (falloffPower)
			*falloffPower = driver ? driver->rolloffFactor.load(std::memory_order_relaxed) : 1.0f;
		if (channelSpec)
			*channelSpec = driver ? driver->channelSpec : MSS_MC_STEREO;
		return nullptr;
	}

	U32 AILCALL AIL_get_timer_highest_delay(void)
	{
		return 0;
	}

	void AILCALL AIL_serve(void)
	{
		// Dropout report BEFORE the lock: the fprintf+fflush is disk I/O, and
		// holding the lock across it was itself a dropout window (each report
		// widened the next miss's odds). UNTHROTTLED -- one timestamped line per
		// miss event, so a listener's "tiny pop at T" can be correlated against a
		// specific miss. AIL_serve runs every ~50 ms, so each line pins its miss
		// to +/-50 ms.
		{
			static std::uint64_t s_lastReportedMisses = 0;
			std::uint64_t const misses = s_callbackLockMisses.load(std::memory_order_relaxed);
			if (misses != s_lastReportedMisses)
			{
				audioDecodeLog("[audio.dropout] +%llu miss(es) (masked by last-block replay), total %llu of %llu blocks",
					static_cast<unsigned long long>(misses - s_lastReportedMisses),
					static_cast<unsigned long long>(misses),
					static_cast<unsigned long long>(s_callbackBlocks.load(std::memory_order_relaxed)));
				s_lastReportedMisses = misses;
			}
		}
		std::vector<HSAMPLE> completed;
		{
			std::lock_guard<std::recursive_mutex> lock(s_mutex);
			for (auto const &entry : s_samples)
				if (entry.second->completed.exchange(false, std::memory_order_acq_rel))
					completed.push_back(entry.first);
		}
		// End-of-sample callbacks fire OUTSIDE the lock: they are engine code that
		// restarts loops and reopens streams (TreeFile I/O), and Miles fired them
		// without our lock too. Handles re-verified unlocked (game thread is the
		// maps' sole mutator; an earlier callback may release a later handle).
		for (HSAMPLE sampleHandle : completed)
		{
			SampleState *sample = findSample(sampleHandle);
			if (!sample)
				continue;
			if (sample->ownerStream)
			{
				StreamState *stream = findStream(sample->ownerStream);
				if (stream && stream->callback)
					stream->callback(sample->ownerStream);
			}
			else if (sample->eosCallback)
			{
				sample->eosCallback(sampleHandle);
			}
		}
	}
}

#endif
