### Fixed

- Fix, in builds with `ZORBA_FOR_ONE_THREAD_ONLY=OFF`, clones of one query
  running in parallel with different implicit timezones: comparisons of dates
  and times used the timezone of whichever clone had opened the shared plan
  last. Comparisons now read the implicit timezone of their own dynamic
  context.
- Fix, in builds with `ZORBA_FOR_ONE_THREAD_ONLY=OFF`, races between clones of
  one query running in parallel: opening the shared plan wrote each
  iterator's state offset, the construction modes of element and document
  constructors and the collators of comparisons, group by and order by into
  the plan. State offsets are now assigned once when the plan is generated,
  and the other values are taken from the static context when the iterator
  is generated; opening a plan no longer writes to it (`openImpl` is
  `const`).
- Fix, in builds with `ZORBA_FOR_ONE_THREAD_ONLY=OFF`, a race on the default
  collator of a static context, which was looked up lazily by its first
  reader; it is now looked up when the default collation is set.

### Changed

- Query plans saved by an earlier version are rejected with ZCSE0012: a saved
  plan now stores the state offsets of its iterators.
