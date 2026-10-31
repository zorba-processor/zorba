(: Regular expressions with back-references, combined with the other
   constructs: category and block escapes, character class subtraction,
   multi-character escapes, case-insensitive back-references, $N in the
   replacement string and non-ASCII input. :)
matches("AbA", "^(\p{Lu})[a-z-[c]]\1$"),
matches("AcA", "^(\p{Lu})[a-z-[c]]\1$"),
matches("&#xE9;&#xE9;", "^(\p{IsLatin-1Supplement})\1$"),
matches("aXa", "^(\w)\W?X\1$"),
matches("ABab", "^(ab)\1$", "i"),
matches("a b a", "^(\i)\s\c\s\1$"),
replace("abcabc", "(a)(b)(c)\1\2\3", "$3$2$1"),
replace("aa", "(a)\1", "$10"),
replace("a.a", "(a)\.\1", "\$1"),
matches("&#x1D518;x&#x1D518;", "^(.)x\1$"),
string-join(analyze-string("x&#xE9;y&#xE9;z", "(&#xE9;)y\1")/*/string(), "|")
