#ifndef MILES_TEST_ENGINE_WORKER_CONTEXT_H
#define MILES_TEST_ENGINE_WORKER_CONTEXT_H

// Plain test-only ABI between modern adapter and engine/STLport translation units.
// Call on the process main thread before launching the adapter control thread.
// Zero means success. Nonzero is terminal for this test process; use ExitProcess.
// Bootstrap remains installed until process exit; no engine-wide teardown runs.
extern "C" int setupEngineProbe();
extern "C" int runEngineWorkerProbe();
// Read-only check on the calling thread; does not install or repair TLS.
extern "C" bool engineProbeThreadReady();

#endif
