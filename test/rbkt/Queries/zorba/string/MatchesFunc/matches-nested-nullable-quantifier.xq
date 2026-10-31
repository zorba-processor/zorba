(: A reluctant quantifier on a group or back-reference that can match the
   empty string made ICU loop forever (QT3 fn-matches-50, test p1030). The
   result must also keep the reluctant (shortest-first) semantics. :)
matches("abcde", "(?:r?)*?r|(.{2,4})"),
matches("abcde", "(?:r?)+?r|(.{2,4})"),
matches("aab", "^(a?)*?b$"),
matches("y", "^(x?)\1*?x$"),
replace("aaa", "^((a?)*?)(a*)$", "[$1][$3]")
