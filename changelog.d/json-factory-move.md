### Changed

- The JSON loader hands the keys and values it parses over to the objects and
  arrays it builds, without changing their reference counts (an atomic
  operation each in the multithreaded build).
