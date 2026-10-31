### Breaking changes

- Regular expressions: `.` matches any character except `\n` and `\r` (it no longer refuses U+0085, U+2028 and U+2029), with the `s` flag it matches exactly one character (`"\r\n"` is two), and `\s` is `[#x20\t\n\r]` (it no longer matches U+00A0, U+0085, U+2028 and the other Unicode spaces), as XSD and F&O 3.0 define them.
- `fn:format-number` raises XPTY0004 for a `$value` that is not numeric (an `xs:string` gave NaN); an `xs:untypedAtomic` value is cast to `xs:double` (it gave NaN).
- A character reference to a character that is not allowed in XML, such as `&#x0;`, in direct element content or an attribute value raises XQST0090.
- A direct element constructor whose end tag does not match its start tag raises XQST0118 instead of XPST0003.
- `fn:format-date`, `fn:format-dateTime` and `fn:format-time` with an Olson timezone name as `$place` adjust the value to that timezone (12:00-10:00 with `America/New_York` formats as 17:00 EST).

### Fixed

- Regular expressions: a reluctant quantifier (`*?`, `+?`) on a group or back-reference that can match the empty string no longer loops until the time limit (QT3 fn-matches-50, `"(?:r?)*?r"`), also without ICU.
- `fn:format-date`, `fn:format-dateTime` and `fn:format-time`: `[Z]`, `[z]` and `[ZN]` output nothing for a value without a timezone (`[ZZ]` still gives `J`); `[ZN]` uses the timezone abbreviation for an Olson name or country code in `$place` (CDT for -05:00 in a US summer) and knows HST and IST.
- `fn:format-number` formats `xs:decimal` and `xs:integer` values exactly instead of through `xs:double`, and `xs:double` values through their shortest digits (`format-number(1E25, '#')` gave rounding noise).
- `fn:lower-case` maps a word-final capital sigma to the final sigma U+03C2 (Unicode Final_Sigma; needs ICU).
