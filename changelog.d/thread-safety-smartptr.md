### Fixed

- Fix data races on the reference counts of the public API handles
  (`SmartPtr` to `SmartObject`: `StaticContext_t`, `XQuery_t`, `Iterator_t`,
  `ItemSequence_t` and the others) in the engine built with
  `-DZORBA_FOR_ONE_THREAD_ONLY=OFF`: copying and releasing handles to one
  object on several threads is now safe in that build. The count is atomic
  there and stays plain in the build for one thread.
