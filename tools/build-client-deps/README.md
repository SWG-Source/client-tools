# Native renderer dependencies

The x64 Direct3d9 projects build original JPEG6b and the repository's STLport 4.5.3 source with VS2013 v120. Win32 projects retain their existing libraries. This does not provide the other client vendor SDKs or establish runtime rendering correctness.

Install VS2013/v120, the Windows 8.1 SDK and the genuine DirectX SDK June 2010. Set `DXSDK_DIR` to the DirectX SDK root. Obtain the [official IJG JPEG6b archive](https://www.ijg.org/files/jpegsrc.v6b.tar.gz); its required SHA256 is:

`75c3ec241e9996504fe02a9ed4d12f16b74ade713972f3db9e65ce95cd27e35d`

Pass `/p:SwgJpegArchive=C:\downloads\jpegsrc.v6b.tar.gz` and, if needed, `/p:SwgPythonExecutable=C:\Python\python.exe` to MSBuild. Python 3.8 or newer is required. Builds do not download dependencies. An explicit standalone invocation can download the pinned archive:

```bat
python tools\build-client-deps\build.py --platform x64 --configuration Release --vcvars "%VS120COMNTOOLS%..\..\VC\vcvarsall.bat" --jpeg-archive C:\downloads\jpegsrc.v6b.tar.gz --download --output C:\build\client-deps\x64\Release
```

Omit `--download` for offline use. Generated libraries, logs, commands, source identities and the retained JPEG README/license go to the output directory. Default project outputs are under `src/compile/deps/v120/<Platform>/<Configuration>`. Override `SwgClientDepsDir` for a private build directory. Do not share one directory across checkouts, architectures or configurations; the builder rejects an output-owner mismatch. Do not edit source/toolchain inputs while builds from that checkout are running. This is an incremental per-checkout build directory, not a shared cross-checkout artifact cache. Failed/overlapping builds report a lock; remove a stale `.build-lock` only after confirming its process stopped.

An absent `owner.json` is created only in an empty output directory (excluding the current `.build-lock`). Unowned contents, malformed/incomplete owner records and mismatched owners fail without deleting existing output. Choose a new empty directory for unowned output. Ownership is published by atomic rename after the staged write is flushed; an interrupted owner publication can leave `owner.pending`, which also blocks adoption. A matching owner is left unchanged and permits retrying an interrupted build after its process has stopped and any stale lock has been handled.

Run the bounded ownership tests with `python -B -m unittest discover -s tools/build-client-deps/tests -v`. They exercise real temporary-directory ownership and entry ordering with the native build boundary mocked; they do not compile or run a renderer.

The builder checks archive identity before extraction, uses all 46 official JPEG library translation units and all 33 bundled STLport library translation units, checks v120 and object architecture, and publishes the completion manifest after both libraries succeed. Cache validation covers source/header and builder content, compiler binaries/frontends, compiler include-tree contents and output archive hashes. Delete `manifest.json` to force a rebuild. CRT libraries are not embedded in these object archives; the consuming renderer link selects and validates its own CRT providers. Logs contain compiler commands/output, never a dumped process environment.

JPEG public headers retain the repository's Windows INT32/FAR adaptations; the original license text is copied unchanged as `JPEG-README.txt`. Distribution must retain the license and attribution requirements in that file. No generated binaries or SDK packages belong in Git.

Native scratch validation covered Debug/Release on both ABIs, opposite-header/library ABI probes, one RGB encoder fixture, 39 legacy/new DxErr names and all six x64 renderer project links. These checks do not prove every JPEG codec path, hostile image safety, Direct3D device creation or rendered frames. The original Win32 diagnostic API remains untouched; x64 uses June 2010's real `DXGetErrorStringA`.
