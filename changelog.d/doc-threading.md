### Changed

- Document the 5.0 threading model: one engine per process (`Zorba::getInstance` is a singleton over a single store), used from one thread at a time or behind the application's own mutex; separate processes are safe. `ZORBA_FOR_ONE_THREAD_ONLY=OFF` remains unsupported. Thread-safety is planned for a 5.x release.
