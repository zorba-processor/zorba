### Fixed

- Fix `icdml:check-integrity-constraint` returning an empty sequence. It now
  returns `true` when the integrity constraint holds and `false` when it does
  not, as its signature `xs:boolean` documents.
