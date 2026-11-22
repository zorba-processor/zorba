### Fixed

- Fix the process aborting (`std::terminate`) when an open result iterator is
  destroyed and closing its query plan fails: the error still goes to the
  query's diagnostic handler, and an exception the handler throws is dropped
  instead of escaping the destructor.
