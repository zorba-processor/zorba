### Fixed

- Fix, in builds with `ZORBA_FOR_ONE_THREAD_ONLY=OFF`, races between threads
  that first use lazily initialized engine state at the same time: the error
  message dictionary, the default full-text stop word lists, the thesaurus
  implementation table, the JSound builtin types, the XQueryX stylesheet and
  the dynamic loader of external modules (now created when the engine starts,
  its library table under a mutex). Each is now initialized once
  (function-local statics, `std::call_once`).
- Fix, in builds with `ZORBA_FOR_ONE_THREAD_ONLY=OFF`, JSound constraints
  compiled on several threads sharing one diagnostics object; each constraint
  now has its own.
- Fix, in builds with `ZORBA_FOR_ONE_THREAD_ONLY=OFF`, XQueryX conversions on
  several threads sharing one error buffer and libxslt's process-wide error
  handler; errors now go to a buffer of the conversion, through the transform
  context.
- Fix, in builds with `ZORBA_FOR_ONE_THREAD_ONLY=OFF`, the namespace string
  pool choosing a hash bucket before taking its mutex, while another thread
  could be resizing the table.
- Fix, in debug builds with `ZORBA_FOR_ONE_THREAD_ONLY=OFF`, iterator ids
  (`--print-item-flow --stable-iterator-ids`) being uninitialized; the id
  counter is now atomic and used in every debug build.

### Changed

- Configuring with `ZORBA_FOR_ONE_THREAD_ONLY=OFF` fails when libxml2 is built
  without thread support (`LIBXML_THREAD_ENABLED`): its error handlers and
  global state are per thread only in threaded builds.
