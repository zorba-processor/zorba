### Fixed

- At O1 and O2, a `for` variable whose domain has at most one item is no
  longer folded into the rest of the FLWOR when its domain can raise an error
  and the variable is not referenced, or a later clause can leave the tuple
  without a result before the domain would be evaluated (for example a later
  `for` clause with an empty domain): the query raises the error that it
  raises at O0. Before, the domain was dropped or never evaluated and the
  error was lost.
- At O1 and O2, `return if (C) then T else E` with an `E` of the empty sequence
  type is turned into `where C return T` only when `E` cannot raise an error.
  Before, the error of `E` was lost for the tuples where `C` is false.
