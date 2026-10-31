(: XQuery F&O 3.0 5.6.1.1: with the "i" flag, characters match if there is a
   default case-insensitive match between them (Unicode case folding), also
   in patterns with a back-reference: KELVIN SIGN folds to "k", LATIN SMALL
   LETTER LONG S to "s". :)
matches("&#x212A;k", "^(k)\1$", "i"),
matches("k&#x212A;", "^(k)\1$", "i"),
matches("&#x17F;s", "^(s)\1$", "i"),
matches("S&#x17F;", "^(s)\1$", "i"),
matches("&#x212A;&#x212A;", "^([a-z])\1$", "i"),
matches("kK", "^([&#x2129;-&#x212A;])\1$", "i"),
matches("&#x212A;k", "^(k)\1$"),
replace("&#x17F;S", "(s)\1", "[$1]", "i")
