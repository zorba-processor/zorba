### Fixed

- Fix a crash (use after free) when a query was compiled against a static
  context whose `StaticContext::loadProlog()` had failed: the functions the
  prolog declared before the error stayed in the context, but their compiled
  bodies were freed with the failed compilation. Those functions are no longer
  declared after the failure; the other declarations of the prolog stay, as
  before, and the failed compilation lives as long as the context.
