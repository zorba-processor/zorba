### Fixed

- A general range index without a key type, or with `xs:anyAtomicType*` as
  its key type, now raises `zerr:ZDTY0012` when a key is a QName, NOTATION,
  binary or `xs:gYear`-like value, as documented, both when the index is
  built and when it is maintained. Release builds used to accept such keys.
- The value probes (`probe-index-point-value`, `probe-index-range-value` and
  their `-skip` forms) and `probe-index-point-general` treat a general index
  whose key type is `xs:anyAtomicType*` as one without a key type: untyped
  search keys and bounds compare as strings, so a range probe with two untyped
  bounds uses both instead of only the lower one, and a range index rejects a
  search key with no ordering with `err:XPTY0004`.
- `probe-index-range-value` without bounds on a general index with domain
  nodes that have several keys returns every domain node once, as documented,
  instead of raising `err:XPTY0004`; with a bound it still raises it.
