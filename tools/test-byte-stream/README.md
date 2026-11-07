# Bounded ByteStream regression checks

Run on Linux with Python 3, MinGW-w64 C++ compilers and Wine:

```
python3 tools/test-byte-stream/run.py --bits 32 --artifacts artifacts/byte-stream-32
python3 tools/test-byte-stream/run.py --bits 64 --artifacts artifacts/byte-stream-64
```

Use `--wine-arch wow64` for Win32 executables on Wine installations that require its new WoW64 mode. Optional `WINEPREFIX32` and `WINEPREFIX64` select reusable dedicated prefixes; otherwise each run creates a temporary prefix.

The runner compiles the checkout's actual `ByteStream.cpp` and Windows `ArchiveMutex.cpp`, with no copied implementation, fake allocator, or header shim. The fixture performs exactly 85 assertions using small initialized buffers: logical read limits, stale/end/default iterators, zero-length operations, self-append, copy-on-write, reserve, and ordinary pool reuse. The candidate must exit zero and print exactly one expected verdict. Logs, input hashes, compiler version, PE architecture and executable hash are retained in the artifact directory. There is no stock-code corruption test, huge allocation, or allocation-failure injection.

This portable run uses MinGW's C++ runtime under Wine. It does not replace native VS2013 builds with bundled STLport, full-client acceptance, or source review of allocation-failure guarantees. It does not establish safety of raw-pointer archive decoders or rollback of nested serializer side effects. Only bound-stream zero-length reads are accepted as no-ops; reading through an unbound iterator remains invalid.
