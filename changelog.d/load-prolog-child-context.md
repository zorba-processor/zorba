### Fixed

- A child (`StaticContext::createChildContext()`) of a static context whose
  prolog was loaded with `StaticContext::loadProlog()` sees the variables of
  that prolog, initialized, as it sees its functions. A query compiled against
  the child, or `invoke()` on the child, raised XQDY0054 for each prolog
  variable with an initializer, and crashed if the parent was released first.
  A prolog loaded into the child extends the parent's.
