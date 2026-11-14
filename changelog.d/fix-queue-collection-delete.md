### Fixed

- `delete` of the static collections DML module on a queue collection now
  deletes the given items when they are all at the beginning of the queue, in
  any order and with repetitions. It raised `zerr:ZDDY0009` for every queue
  delete; it still raises it when an item is not at the beginning. The module
  documentation of `delete` now lists `zerr:ZDDY0004`, `zerr:ZDDY0007` and
  `zerr:ZDDY0009` instead of `zerr:ZDDY0006`.
