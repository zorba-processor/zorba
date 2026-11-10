### Fixed

- Fix hash indexes, hash joins, `fn:distinct-values` and `group by` on date
  and time values, which gave every value the same hash code, so each lookup
  compared the key with every date/time entry of the table. Values are now
  hashed under the implicit timezone that the comparison uses, so a value
  without a timezone still matches an equal value with a timezone.
- Fix unordered maps and value indexes with `xs:anyAtomicType` keys, which
  treated date/time keys of different types that denote the same instant (for
  example `xs:date("2013-01-13Z")` and `xs:dateTime("2013-01-13T00:00:00Z")`)
  as one key; `eq` does not compare them, so they are now different keys.
