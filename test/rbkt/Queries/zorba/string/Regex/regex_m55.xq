(: ^ outside a character class is always an anchor; inside one it is an
   ordinary character unless it comes first. Without the m flag, $ matches
   only at the very end of the string, wherever it appears in the regex. :)
(fn:matches("a", "^^a"),
 fn:matches("x", "a^b|x"),
 fn:matches("a^", "^[a^]+$"),
 fn:matches("ab&#10;", "ab$|q"),
 fn:matches("ab&#10;", "ab$|q", "m"))
