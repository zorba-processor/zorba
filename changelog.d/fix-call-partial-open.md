### Fixed

- Fix a memory leak when opening the body of a function call, a dynamic
  function call, or the function passed to `fn:for-each-pair`, `fn:fold-left`
  or `fn:fold-right` fails partway, for example when memory runs out: the
  iterator states created before the failure are now destroyed.
