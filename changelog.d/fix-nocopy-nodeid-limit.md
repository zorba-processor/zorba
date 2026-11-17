### Fixed

- At O1 and O2, a constructor that placed its content without a copy (the
  no-copy optimization) did not raise `err:XPDY0130` where copying it raises it
  at O0: a tree whose node ids would exceed the limit, e.g. 1100 levels of
  `<a>{ $content }</a>`, was built. Placing content without a copy now
  computes the node ids a copy would get, raises the same error with the same
  depth at the constructor, and works up to the same depth as at O0.
