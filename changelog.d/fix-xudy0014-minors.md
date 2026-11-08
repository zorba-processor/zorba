### Fixed

- Fix `err:XUDY0014` in a copy modify expression reporting, when several
  update targets are outside the copy, a location that depended on the order
  of the store's internal table; it now reports the first one in the query
  text.
- Fix the message of `err:XUDY0014` saying "node" for an update whose target is
  a collection, an index, an integrity constraint or a document; it now says
  what the target is.
- The modify clause of a copy modify expression also raises `XUDY0014` for
  `icddl:deactivate`, which the earlier entry on collection, document, index
  and integrity-constraint updates (activation only) did not list.
