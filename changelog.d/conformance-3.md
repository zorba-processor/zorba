### Fixed

- `xs:dateTime`/`xs:time` with more than 6 fractional-second digits are truncated to microseconds; `"23:59:59.999999999"` no longer prints as `59.1` with `seconds-from-dateTime` returning 60.
- A negative `xs:dayTimeDuration` keeps its sign in `*` and `div` (`-PT1S * 1.5` gave `PT1.5S`, `-PT1S div PT1S` gave 1).
- Multiplying or dividing a duration into a value too large to represent raises FODT0002 instead of the internal error ZXQP0003.
- Casting to `xs:token` collapses whitespace instead of failing with ZXQP0003 on leading or trailing whitespace; casts to `xs:language`, `xs:NMTOKEN`, `xs:Name`, `xs:NCName`, `xs:ID`, `xs:IDREF` and `xs:ENTITY` collapse whitespace before validating (`" true " cast as xs:language`).
- `fn:translate`: when a character occurs more than once in `$mapString`, the first occurrence wins (`translate('aaa', 'aa', 'bc')` is `bbb`).
- `fn:upper-case` and `fn:lower-case` use the full Unicode case mappings (`upper-case('ß')` is `SS`; titlecase letters such as U+01C5 are mapped).
- `fn:escape-html-uri` escapes all non-ASCII characters.
- `fn:resolve-uri("a", "http://a")` returns `http://a/a` (RFC 3986 merge with an empty base path).
- `fn:namespace-uri-for-prefix` and `fn:resolve-QName` know the `xml` prefix; `fn:namespace-uri-for-prefix` returns `xs:anyURI`.
- `fn:round-half-to-even` on `xs:double`/`xs:float` with a precision beyond the value's last digit returns the value (`round-half-to-even(4.9e-324, 400)` gave 0).
- `castable as xs:QName` returns false for an invalid string literal instead of raising FORG0001.
- Casting a string without digits (whitespace only, `"+"`, `"."`) to `xs:decimal` raises FORG0001 instead of returning 0.
- Casting to `xs:double`/`xs:float` rejects trailing characters and hexadecimal forms (`"0x1A"` gave 30).
- `lt`, `gt`, `le`, `ge` and `<`, `>`, `<=`, `>=` on `xs:gYear`, `xs:gYearMonth`, `xs:gMonth`, `xs:gMonthDay` and `xs:gDay` raise XPTY0004.
- `fn:max`/`fn:min` return a NaN of the promoted type (`max((0e0, xs:float('NaN')))` is an `xs:double`).
- `fn:format-integer` formats -9223372036854775808, and ordinals of 21, 22, 23 (and so on) end in st, nd, rd.
- Casting a negative decimal, float or double at or below -1 to an unsigned integer type raises FORG0001 instead of FOCA0002.
- The `zorba` command applies serialization parameters declared in the query prolog (`declare option output:method "text"`); `-z` and the `--serialize-*` options still override them.
