(: F&O 3.0 5.6.1: a back-reference to a capturing group that did not
   participate in the match matches the empty string (QT3 fn-matches-50,
   tests p294, p295 and p303). A group inside a repeated group participates
   only if it matched in the current iteration. :)
matches("x", "(a)|\1"),
matches("a", "(?:(b)?a)\1"),
matches("babadad", "^((.)?a\2)+$"),
matches("a", "^(a)?a\1$"),
matches("A", "^(b)?a\1$", "i"),
matches("a&#10;b", "^(c)?b\1$", "m"),
matches("a&#10;", "^(x)?a.\1$", "s"),
matches("a", "^ ( b ) ? a \1 $", "x"),
matches("abab", "^(ab)\1$"),
matches("abac", "^(ab)\1$"),
replace("xa", "(?:(b)|x)a\1", "[$1]"),
string-join(tokenize("1-2--3", "(x)?-\1"), "|"),
string-join(analyze-string("cab", "(c)?(a)b\1")//fn:group/@nr/string(), "|"),
string(analyze-string("cab", "(c)?(a)b\1")/fn:match)
