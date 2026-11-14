### Fixed

- Fix creating an inline function item raising XPDY0002 when a block variable
  without a value is in scope: an inline function item now captures only the
  variables its body uses.
