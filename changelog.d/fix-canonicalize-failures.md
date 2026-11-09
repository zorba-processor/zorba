### Fixed

- `x:canonicalize` now reports memory exhaustion as memory exhaustion instead
  of `FOCZ0001`, and no longer crashes when libxml2 cannot produce the
  canonical form. An input whose canonical form cannot be built, such as one
  with a relative namespace URI, raises `FOCZ0001` with the reason libxml2
  gives. `FOCZ0001` is otherwise kept for input that is not well formed.
