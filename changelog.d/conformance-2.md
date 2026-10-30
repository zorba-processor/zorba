### Breaking changes

- Regular expressions: patterns that are invalid in XSD/XPath but that ICU accepted now raise FORX0002: a literal `-` that is neither first nor last in a character group (`[a-h-o-z]`, `[0-9-.]`), a character class subtraction that is not last in its group (`[a-[b] ]`), and block escapes whose name cannot be an XSD block name (`\p{IsBasic-Latin}`).
- `fn:parse-xml-fragment` (and the xml module's parse-external-parsed-entity option): a text declaration without an encoding declaration (`<?xml version='1.0'?>`) is FODC0006, as required for an external parsed entity.
- `group by` requires a grouping variable in XQuery and JSONiq: `group by $x mod 10` is XPST0003; write `group by $k := $x mod 10`.

### Fixed

- xs:float and xs:double are cast to strings with the shortest digit sequence that round-trips: `xs:float(1.999)` gave `2`, `xs:float(1.0000001)` gave `1`, and doubles lost or gained digits (`3.1415926535897931`).
- `math:acos(-0)` returns +π/2 instead of -π/2; the math module's `asinh` and `atanh` keep the sign of zero.
- Regular expressions: `^` outside a character class is always an anchor (`fn:matches("a", "^^")` is true); without the `m` flag `$` matches only at the end of the string wherever it appears; `[\s-]` and `[^-a]` are accepted.
- Serialization no longer inserts a space between a text node and an adjacent atomic value.
- `fn:unparsed-text` and `fn:unparsed-text-lines` raise FOUT1170 instead of FODC0005 for an invalid `$href`.
- zq:delete-query no longer frees a query whose function items are still in use by the outer query (crash in the QT3 FOTS driver, inline-fn-032/-033).
- Annotations on inline function expressions and function tests are parsed (XQuery 3.0); `%public`/`%private` on an inline function raise XQST0125.
- URIQualifiedNames (`Q{uri}local`) are accepted in computed element and attribute constructors, as literal names and as computed name strings.
