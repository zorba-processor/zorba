### Fixed

- At O1 and O2, a FLWOR with an empty `for` domain after a `for` clause over an
  external variable without a default value, or over the context item, raises
  XPDY0002 when that variable or the context item is not bound, as it does at
  O0. Before, the FLWOR was replaced by the empty sequence and the error was
  lost.
