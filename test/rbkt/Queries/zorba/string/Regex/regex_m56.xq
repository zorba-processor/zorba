(: Valid uses of '-' and block escapes in character classes. :)
(fn:matches("-", "^[-a]$"),
 fn:matches("-", "^[^-a]$"),
 fn:matches("-", "^[a-]$"),
 fn:matches("-", "^[\s-]$"),
 fn:matches("b", "^[a-z-[aeiou]]$"),
 fn:matches("&#xE9;", "^\p{IsLatin-1Supplement}$"),
 fn:matches("&#x100;", "^\p{IsLatinExtended-A}$"),
 fn:matches("a", "^\p{IsBasicLatin}$"))
