### Fixed

- At O1 and O2, a `for` variable whose domain has at most one item is no
  longer folded into the rest of the FLWOR when its domain can raise an error
  and the variable is not referenced, or a later clause can leave the tuple
  without a result (for example a later `for` clause with an empty domain or
  a `where` clause). Before, the domain was dropped or never evaluated and
  the error that the query raises at O0 was lost. A folded domain whose
  reference is in a position evaluated lazily can still go unevaluated.
- At O1 and O2, `return if (C) then T else E` with an `E` of the empty sequence
  type is turned into `where C return T` only when `E` cannot raise an error.
  Before, the error of `E` was lost for the tuples where `C` is false.
