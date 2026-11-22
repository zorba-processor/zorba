### Fixed

- Fix `pop_nodestack` in the translator returning an uninitialized pointer
  when asked to pop zero entries; it now returns null.
- Fix two debug assertions in the QName pool free list that assigned instead
  of comparing, so they checked nothing.
