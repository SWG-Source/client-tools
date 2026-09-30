#ifndef STARTUP_BRIDGE23_BACKEND_H
#define STARTUP_BRIDGE23_BACKEND_H
#include "../dispatch/registry_resolver.h"
#include "../version/session_version_host.h"
#include "reply.h"
#include <Mss.h>
#include "../upload/upload_state.h"
#include "../eos/host_eos.h"
#include <type_traits>
static_assert(std::is_same<decltype(&::AIL_WAV_info),
    S32 (AILCALL *)(const void *, AILSOUNDINFO *)>::value,"exact native WAV_info declaration");
// Same single Backend owner as the exercised live candidate, narrowed to this
// startup/metadata/driver slice. No SessionLifecycle or fixture expectations.
struct Backend {
    MilesTransport::ResourceRegistry registry;
    MilesHost::RegistryResolver resolver;
    MilesStartup::SessionInputs metadataInputs;
    HMODULE module;
    HDIGDRIVER driver;
    MilesWire::Handle driverId;
    // Serialized open transaction retained by the heap-owned Backend on failure.
    MilesTransport::ResourceRegistry::Reservation streamReservation;
    HSTREAM pendingStream;
    bool streamOpenPending;
    MilesHostUpload106::UploadState imageUpload;
    bool started, shutdown;
    unsigned metadataInvocations;
    explicit Backend(const char *expectedPath, uint32_t imageBudgetBytes)
        : resolver(registry), module(0), driver(0), pendingStream(0), streamOpenPending(false),
          imageUpload(registry,imageBudgetBytes),
          started(false), shutdown(false),
          metadataInvocations(0) {
        require(MilesImage93::validBudget(imageBudgetBytes), "explicit upload byte budget");
        driverId = MilesWire::Handle();
        require(expectedPath && *expectedPath, "expected vendor module path");
        char expectedAbsolute[MAX_PATH] = {};
        const DWORD expectedLength = GetFullPathNameA(expectedPath, MAX_PATH, expectedAbsolute, 0);
        require(expectedLength && expectedLength < MAX_PATH, "normalize expected vendor module path");
        require(GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                                   reinterpret_cast<LPCSTR>(&::AIL_startup), &module) != 0,
                "hold actual imported vendor module");
        char loaded[MAX_PATH] = {};
        DWORD n = GetModuleFileNameA(module, loaded, MAX_PATH);
        char loadedAbsolute[MAX_PATH] = {};
        const DWORD loadedLength = n && n < MAX_PATH ?
            GetFullPathNameA(loaded, MAX_PATH, loadedAbsolute, 0) : 0;
        if (!loadedLength || loadedLength >= MAX_PATH) {
            FreeLibrary(module);
            module = 0;
            throw std::runtime_error("normalize imported vendor module path");
        }
        if (_stricmp(loadedAbsolute, expectedAbsolute)) {
            FreeLibrary(module);
            module = 0;
            throw std::runtime_error("imported module path mismatch");
        }
        printf("loaded_dll=%s\n", loaded);
    }
    ~Backend() {
        if (started) {
            ::AIL_shutdown();
            puts("FAIL emergency vendor cleanup");
        }
        if (module)
            FreeLibrary(module);
    }
    bool uploadFailed() const { return imageUpload.failed(); }
    bool uploadActive() const { return imageUpload.active(); }
    void observeUploadRefusal(uint32_t status) { imageUpload.observeRefusal(status); }
    static int32_t bindImageNative(uint32_t opcode,void *sample,const char *suffix,
                                  const void *image,uint32_t size,int32_t block) {
        const S32 status=opcode==MilesWire::AIL_set_named_sample_file ?
            ::AIL_set_named_sample_file(static_cast<HSAMPLE>(sample),suffix,image,size,block) :
            ::AIL_set_sample_file(static_cast<HSAMPLE>(sample),image,block);
        int32_t result=0;
        static_assert(sizeof(status)==sizeof(result),"native sample file result width");
        std::memcpy(&result,&status,sizeof(result));return result;
    }
    static MilesHostUpload106::QueryResult queryImageNative(uint32_t opcode,const void *data,uint32_t size) {
        MilesHostUpload106::QueryResult out={};
        S32 status=0;
        if(opcode==MilesWire::AIL_WAV_info) {
            AILSOUNDINFO info={};
            status=::AIL_WAV_info(data,&info);
            if(status) {
                out.value[0]=static_cast<uint32_t>(info.format);
                out.value[1]=static_cast<uint32_t>(info.bits);
                out.value[2]=static_cast<uint32_t>(info.channels);
                out.value[3]=info.data_len;out.value[4]=info.rate;
                out.value[5]=info.samples;out.value[6]=info.block_size;
            }
        } else {
            require(opcode==MilesWire::AIL_file_type,"known readonly image query");
            status=::AIL_file_type(data,size);
        }
        static_assert(sizeof(status)==sizeof(out.status),"readonly query status width");
        std::memcpy(&out.status,&status,sizeof(status));
        return out;
    }
    StartupBridge::OwnedReply execute(const MilesWire::Header &h, const MilesWire::Call &c,
                                      const std::vector<unsigned char> &frame) {
        using namespace StartupBridge;
        require(!streamOpenPending, "uncertain stream open cannot resume");
        OwnedReply uploadReply;
        if(imageUpload.intercept(h.opcode,c,frame,started,shutdown,
                                &Backend::queryImageNative,&require,uploadReply,&Backend::bindImageNative))return uploadReply;
        OwnedReply out;
        out.result.transport_status = InvalidFields;
        // Narrow native25 non-callback slice; reuse reviewed SDK dispatcher.
        switch (h.opcode) {
        case MilesWire::AIL_start_sample:
        case MilesWire::AIL_stop_sample:
        case MilesWire::AIL_sample_status:
        case MilesWire::AIL_set_sample_loop_count:
        case MilesWire::AIL_set_sample_loop_block:
        case MilesWire::AIL_set_sample_ms_position:
        case MilesWire::AIL_set_sample_position:
        case MilesWire::AIL_sample_position:
        case MilesWire::AIL_sample_reverb_levels:
        case MilesWire::AIL_set_sample_3D_position:
        case MilesWire::AIL_set_sample_3D_velocity_vector:
        case MilesWire::AIL_set_sample_3D_distances:
        case MilesWire::AIL_set_sample_occlusion:
        case MilesWire::AIL_set_sample_obstruction:
            if (c.target.kind != MilesWire::OwnedSample) {
                out.result.transport_status = InvalidResource;
                return out;
            }
            // fall through to the shared lifecycle and real SDK dispatcher
        // The real dispatcher validates OwnedSample | BorrowedSample for exactly
        // these five controls, including a live stream parent via the resolver.
        case MilesWire::AIL_set_sample_playback_rate:
        case MilesWire::AIL_sample_playback_rate:
        case MilesWire::AIL_set_sample_volume_levels:
        case MilesWire::AIL_sample_volume_levels:
        case MilesWire::AIL_set_sample_reverb_levels:
        // Existing stream controls; lifetime cases are handled below.
        case MilesWire::AIL_start_stream:
        case MilesWire::AIL_set_stream_loop_count:
        case MilesWire::AIL_set_stream_loop_block:
        case MilesWire::AIL_stream_status:
        case MilesWire::AIL_set_stream_ms_position:
        case MilesWire::AIL_stream_ms_position:
        case MilesWire::AIL_sample_ms_position:
        case MilesWire::AIL_end_sample:
        case MilesWire::AIL_set_listener_3D_position:
        case MilesWire::AIL_set_listener_3D_velocity_vector:
        case MilesWire::AIL_set_listener_3D_orientation:
        case MilesWire::AIL_set_3D_rolloff_factor:
        case MilesWire::AIL_active_sample_count:
        case MilesWire::AIL_digital_CPU_percent:
        case MilesWire::AIL_digital_latency:
        case MilesWire::AIL_get_timer_highest_delay:
        case MilesWire::AIL_file_error:
        case MilesWire::AIL_serve:
        case MilesWire::AIL_room_type:
        case MilesWire::AIL_set_room_type:
            if (!started || shutdown) {
                out.result.transport_status = LifecycleRefused;
                return out;
            }
            switch (MilesHost::dispatch(h.opcode, c, out.result, resolver)) {
            case MilesHost::Complete: out.result.transport_status = Success; break;
            case MilesHost::Unsupported: out.result.transport_status = Unsupported; break;
            case MilesHost::InvalidResource: out.result.transport_status = InvalidResource; break;
            case MilesHost::InvalidFields: out.result.transport_status = InvalidFields; break;
            }
            return out;
        default:
            break;
        }
        if(h.opcode==MilesWire::AIL_register_EOS_callback || h.opcode==MilesWire::AIL_register_stream_callback) {
            const bool sample=h.opcode==MilesWire::AIL_register_EOS_callback;
            if(c.reserved||c.output_mask||c.bytes.offset||c.bytes.length||c.text.offset||c.text.length||!nullHandle(c.resource))return out;
            for(unsigned i=0;i<8;++i)if(c.value[i])return out;
            if(c.callback&&(sample?(c.callback>64):(c.callback<65||c.callback>128)))return out;
            if(!started||shutdown){out.result.transport_status=LifecycleRefused;return out;}
            void *local=0;
            if(!registry.resolve(c.target,sample?MilesWire::OwnedSample:MilesWire::Stream,local)){
                out.result.transport_status=InvalidResource;return out;
            }
            out.result.callback=MilesHostEos::registerCallback(local,c.target,c.callback);
            out.result.transport_status=Success;return out;
        }
        // Callback identities remain live throughout native close and its synchronous completion join.
        if (h.opcode == MilesWire::AIL_open_stream ||
            h.opcode == MilesWire::AIL_stream_sample_handle ||
            h.opcode == MilesWire::AIL_close_stream) {
            const bool opening = h.opcode == MilesWire::AIL_open_stream;
            if (c.callback || c.reserved || c.output_mask || c.bytes.offset || c.bytes.length ||
                !nullHandle(c.resource)) return out;
            for (unsigned i = opening ? 1u : 0u; i < 8; ++i)
                if (c.value[i]) return out;
            if (opening) {
                if (!frame.data() || !c.text.length || c.text.offset > frame.size() ||
                    c.text.length > frame.size() - c.text.offset) return out;
                const unsigned char *name = frame.data() + c.text.offset;
                if (name[c.text.length - 1] || std::memchr(name, 0, c.text.length - 1)) return out;
            } else if (c.text.offset || c.text.length) return out;
            if (!started || shutdown) {
                out.result.transport_status = LifecycleRefused;
                return out;
            }
            void *local = 0;
            if (!registry.resolve(c.target, opening ? MilesWire::Driver : MilesWire::Stream, local)) {
                out.result.transport_status = InvalidResource;
                return out;
            }
            if (opening) {
                S32 streamMem = 0;
                static_assert(sizeof(streamMem) == sizeof(c.value[0]), "native signed stream memory width");
                std::memcpy(&streamMem, &c.value[0], sizeof(streamMem));
                require(registry.reserve(MilesWire::Stream, c.target, streamReservation),
                        "stream tracking capacity before native open");
                streamOpenPending = true; // do not cancel after an unknown native effect
                pendingStream = ::AIL_open_stream(static_cast<HDIGDRIVER>(local),
                    reinterpret_cast<const char *>(frame.data() + c.text.offset), streamMem);
                if (pendingStream) {
                    require(registry.publish(streamReservation, pendingStream, out.result.resource),
                            "publish reserved stream identity");
                    pendingStream = 0; // registry retains the native identity before reply
                } else registry.cancel(streamReservation); // confirmed native null only
                streamOpenPending = false;
            } else if (h.opcode == MilesWire::AIL_stream_sample_handle) {
                const HSAMPLE sample = ::AIL_stream_sample_handle(static_cast<HSTREAM>(local));
                if (sample)
                    require(registry.insertBorrowed(c.target, sample, out.result.resource),
                            "retain stable borrowed identity; parent remains owned on failure");
                out.result.value[0] = c.target.kind;
                out.result.value[1] = c.target.slot;
                out.result.value[2] = c.target.generation;
            } else {
                require(registry.beginClose(c.target), "close stream identity before native effect");
                ::AIL_close_stream(static_cast<HSTREAM>(local));
                MilesHostEos::released(local,c.target);
                require(registry.retire(c.target), "retire stream and aliases after native return");
            }
            out.result.transport_status = Success;
            return out;
        }
        if (h.opcode == MilesWire::AIL_allocate_sample_handle ||
            h.opcode == MilesWire::AIL_release_sample_handle) {
            if (c.callback || c.reserved || c.output_mask || c.bytes.offset || c.bytes.length ||
                c.text.offset || c.text.length || !nullHandle(c.resource)) return out;
            for (unsigned i = 0; i < 8; ++i) if (c.value[i]) return out;
            if (!started || shutdown) {
                out.result.transport_status = LifecycleRefused;
                return out;
            }
            void *local = 0;
            const bool allocating = h.opcode == MilesWire::AIL_allocate_sample_handle;
            if (!registry.resolve(c.target, allocating ? MilesWire::Driver : MilesWire::OwnedSample,
                                  local)) {
                out.result.transport_status = InvalidResource;
                return out;
            }
            if (allocating) {
                MilesTransport::ResourceRegistry::Reservation reservation;
                require(registry.reserve(MilesWire::OwnedSample, c.target, reservation),
                        "sample tracking capacity before vendor allocation");
                PendingSample pending; // constructed before vendor work; cannot allocate
                pending.sample = ::AIL_allocate_sample_handle(static_cast<HDIGDRIVER>(local));
                if (pending.sample) {
                    require(registry.publish(reservation, pending.sample, out.result.resource),
                            "publish reserved sample identity");
                    pending.sample = 0; // registry now owns shutdown/release tracking
                }
                // Null allocation cancels its unpublished reservation on scope exit.
            } else {
                ::AIL_release_sample_handle(static_cast<HSAMPLE>(local));
                MilesHostEos::released(local,c.target);
                imageUpload.releasedSample(c.target);
                require(registry.retire(c.target), "retire sample after confirmed release");
            }
            out.result.transport_status = Success;
            return out;
        }
        if (metadata(h.opcode)) {
            const MilesStartup::Status shape = MilesStartup::validate(h.opcode, c, bytes(frame));
            if (shape != MilesStartup::Complete) {
                out.result.transport_status = mapMetadata(shape);
                return out;
            }
            if (shutdown || (!started && h.opcode != MilesWire::AIL_set_redist_directory)) {
                out.result.transport_status = LifecycleRefused;
                return out;
            }
            MilesStartup::Reply local;
            ++metadataInvocations;
            const MilesStartup::Status status =
                MilesStartup::dispatch(h.opcode, c, bytes(frame), local, resolver, metadataInputs);
            out.result = local.result;
            out.result.transport_status = mapMetadata(status);
            out.text.swap(local.text);
            return out;
        }
        if (h.opcode == MilesWire::SessionVersion) {
            MilesWire::Header checked = {}; uint32_t capacity = 0;
            if (!MilesSessionVersion::validateQuery(bytes(frame), checked, capacity))
                return out;
            if (shutdown) {
                out.result.transport_status = LifecycleRefused;
                return out;
            }
            std::vector<unsigned char> encoded;
            require(MilesSessionVersion::queryNativeVersion(bytes(frame), encoded),
                    "version operation has unsupported native buffer outcome");
            require(decodeReply(bytes(encoded), h, out), "owned version adapter result");
            return out;
        }
        if (c.callback || c.reserved || c.output_mask || c.bytes.length || c.text.length ||
            !nullHandle(c.target) || !nullHandle(c.resource))
            return out;
        unsigned values = 0;
        switch (h.opcode) {
        case MilesWire::AIL_open_digital_driver:
            values = 4;
            break;
        case MilesWire::AIL_startup:
        case MilesWire::AIL_shutdown:
        case MilesWire::AIL_lock:
        case MilesWire::AIL_unlock:
        case MilesWire::SessionClose:
            break;
        default:
            out.result.transport_status = Unsupported;
            return out;
        }
        for (unsigned i = values; i < 8; ++i)
            if (c.value[i])
                return out;
        out.result.transport_status = LifecycleRefused;
        if (h.opcode == MilesWire::AIL_startup) {
            if (started || shutdown)
                return out;
            const S32 value = ::AIL_startup();
            started = value != 0;
            out.result.return_bits = static_cast<uint32_t>(value);
            out.result.transport_status = Success;
            printf("vendor startup=%ld\n", value);
            return out;
        }
        if (h.opcode == MilesWire::SessionClose) {
            if (!started && shutdown && !driver)
                out.result.transport_status = Success;
            return out;
        }
        if (!started || shutdown)
            return out;
        if (h.opcode == MilesWire::AIL_lock || h.opcode == MilesWire::AIL_unlock) {
            // Genuine native counter operations. Neither return proves producer quiescence.
            if (h.opcode == MilesWire::AIL_lock) ::AIL_lock();
            else ::AIL_unlock();
            out.result.transport_status = Success;
            return out;
        }
        if (h.opcode == MilesWire::AIL_open_digital_driver) {
            if (driver)
                return out;
            MilesTransport::ResourceRegistry::Reservation reservation;
            require(registry.reserve(MilesWire::Driver, MilesWire::Handle(), reservation),
                    "driver tracking capacity before vendor open");
            // Decode signed fields without implementation-defined U32-to-S32 narrowing.
            const S32 bits = static_cast<S32>(MilesStartup::signedValue(c.value[1]));
            const S32 channels = static_cast<S32>(MilesStartup::signedValue(c.value[2]));
            driver = ::AIL_open_digital_driver(c.value[0], bits, channels, c.value[3]);
            if (driver)
                require(registry.publish(reservation, driver, driverId),
                        "publish reserved driver identity");
            out.result.resource = driverId;
            out.result.transport_status = Success;
            return out;
        }
        if (h.opcode == MilesWire::AIL_shutdown) {
            ::AIL_shutdown();
            MilesHostEos::shutdownComplete();
            imageUpload.shutdownComplete();
            started = false;
            shutdown = true;
            if (driver) {
                require(registry.retire(driverId), "retire driver after shutdown");
                driver = 0;
            }
            printf("vendor shutdown complete; retained directory bytes=%u\n",
                   static_cast<unsigned>(metadataInputs.retainedBytes()));
            out.result.transport_status = Success;
            return out;
        }
        return out;
    }

  private:
    struct PendingSample {
        HSAMPLE sample;
        PendingSample() : sample(0) {}
        ~PendingSample() { if (sample) ::AIL_release_sample_handle(sample); }
      private:
        PendingSample(const PendingSample &);
        PendingSample &operator=(const PendingSample &);
    };
    Backend(const Backend &);
    Backend &operator=(const Backend &);
};
#endif
