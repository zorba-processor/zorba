### Fixed

- Fix general index probes with several search keys, which carried state from
  one key to the next: after an untyped key, later typed keys skipped untyped
  index entries, and the matches of an earlier key were returned again under
  the rules of a later key, which could add `xs:decimal` entries that only
  share its `xs:double` image.
- Fix a crash of `idml:probe-index-range-general` with several bounds when the
  first bound has matches, and a crash when the same range probe runs again,
  for example once per iteration of a `for` clause.
