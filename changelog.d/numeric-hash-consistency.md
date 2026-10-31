### Fixed

- Hash xs:integer and xs:decimal values beyond 2^53 on the xs:double they are promoted to, so that values equal under value comparison (e.g. 9007199254740993 and 9.007199254740992E15) are recognized as duplicates by `fn:distinct-values`, `group by` and the hash tables built on item hashes.
- `group by` with several grouping keys no longer treats two tuples as equal because their first keys are both NaN.
