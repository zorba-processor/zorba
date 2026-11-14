### Fixed

- A serialization that ran out of memory while its emitter made its children
  iterators, or added one for an element nested deeper than 8 levels, left
  those iterators allocated.
