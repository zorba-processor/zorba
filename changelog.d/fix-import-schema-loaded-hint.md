### Fixed

- An `import schema` of a namespace that is already loaded no longer resolves
  its location hints: a hint that cannot be resolved no longer raises
  `XQST0059`, and a hint that would be disregarded is not fetched.
