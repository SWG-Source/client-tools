# Archive payload preflight fixtures

```
python3 tools/test-archive-decoders/run.py --bits 32 --artifacts artifacts/decoder-32
python3 tools/test-archive-decoders/run.py --bits 64 --artifacts artifacts/decoder-64
```

Requires Python3, MinGW-w64 C++ and Wine; add `--wine-arch wow64` for a Wine installation requiring that mode for Win32. Compiles the checkout's actual ByteStream, Windows ArchiveMutex and UnicodeArchive implementations with no substitute headers or allocator. Artifacts retain source/toolchain hashes, executable architecture, logs and exact34-check verdict gating.

Candidate-only initialized small-buffer fixtures cover valid short/long-form string headers, embedded zeros, empty strings and byte streams, ByteStream append semantics, Unicode at an odd payload offset, incomplete headers, and declared payload sizes beyond remaining input. Rejected payload lengths must preserve destination contents; successfully read headers remain consumed. No malformed-input execution against old code or huge allocation is performed.

This is payload bounds coverage, not a generic transaction guarantee. Earlier members in an outer message can already have changed; allocation exceptions are not injected. Unicode writer byte-count multiplication is outside this change. Portable MinGW/Wine tests do not substitute for native v120/bundled-STLport validation or full-client acceptance.
