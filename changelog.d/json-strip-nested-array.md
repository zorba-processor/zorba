### Fixed

- Fix `jsoniq-strip-top-level-array` also stripping an array nested in a
  top-level object: `{"a":[1,2]}` returned no object.
