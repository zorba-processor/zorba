### Fixed

- Fix document order around 31 levels of nesting: a node id that used all
  8 bytes kept inline compared its inline flag bit as data, so `<<`, `>>`,
  `is`, duplicate removal and ancestor tests could be wrong between such a
  node and its deeper descendants (`$s[30] << $s[31]` was false at O0, and
  O0 and O1 could disagree on duplicate removal).
- Fix the decoding of node ids at six bytes of the encoding (0000 0000,
  0001 1111, 0101 0101, 0101 0110, 1010 0100, 1010 0111): ids such as
  1.1.1.3 decoded as 1.1.1, with effects on node levels, sibling and parent
  tests and on the ids given to inserted nodes. Ids with components below
  -1 118 484 did not decode at all; an odd one at the end of an id crashed.
- Fix node levels and parent and sibling tests for ids with negative odd
  components (made by inserting before the first child), which did not
  count as levels.
