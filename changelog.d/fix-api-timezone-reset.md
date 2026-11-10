### Fixed

- Fix the implicit timezone and the current dateTime that a host sets on the
  dynamic context (`DynamicContext::setImplicitTimezone`,
  `DynamicContext::setCurrentDateTime`, `zorba --timezone`) being replaced by
  the timezone and clock of the host at each execution of the query. A value
  the host sets now applies to every execution; a value it does not set is
  still taken from the host at the start of each execution. The API
  documentation now states that the timezone is in minutes.
