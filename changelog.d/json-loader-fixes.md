### Fixed

- Fix a UTF-8 byte order mark at the start of a text resource staying in the
  string: `utf8::BOM` was four bytes long, so `skip_utf8_bom` never found a mark,
  and each read of a streamed string went back to position 0. `jn:parse-json`
  raised `JNDY0021`, `csv:parse` and `dir:open` had it in the first key, and the
  string value started with U+FEFF. `file:read-text`, `file:read-text-lines`,
  `fn:unparsed-text`, `fn:unparsed-text-lines` and `fetch:content` now skip the
  mark. Only when the resource is decoded as UTF-8:
  in `UTF-16LE` and `UTF-16BE` a leading U+FEFF is text. `base64:decode` keeps it. Over HTTP these functions decode the resource as
  ISO-8859-1 (existing behavior), so a mark there arrives as three characters.
- `jn:parse-json` ignores a byte order mark at the start of its input, as
  `fn:parse-json` does in F&O 3.1.
- `byte-order-mark="yes"`: write the two octets of the mark in the right order for
  `UTF-16LE` and `UTF-16BE` (it wrote four, swapped), no NUL after the UTF-8 mark,
  no second mark for `UTF-16`, for `encoding` in any case, and with the `text`
  method too. `fn:serialize` skips encoding, so its string has no mark.
- `fn:unparsed-text-lines` and `file:read-text-lines`: lines also end with x0D, and
  a last empty line does not count, as F&O defines it. An empty file has no lines.
- The `text` output method of a streamed string no longer goes back to position 0.
- JSON serialization of `-INF` no longer loses the sign.
