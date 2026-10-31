(: With the q flag the pattern is literal: "a?" does not match the empty
   string and a lone backslash is valid (QT3 fn-replace-35, -50). :)
(fn:replace("a?bracadabra?", "a?", "\$1", "q"),
 fn:replace("a\b\c", "\", "\\", "q"),
 fn:matches("x.y", ".", "q"),
 fn:tokenize("a+b+c", "+", "q"))
