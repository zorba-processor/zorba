(: XQuery 3.0 F&O 5.6.1: without the 's' flag, '.' matches every character
   except #x0A and #x0D; XSD F.1.1: \s is [#x20\t\n\r]. ICU's own '.' and \s
   differ on U+0085, U+00A0, U+2028 and U+2029. :)
matches("&#x85;", "^.$"),
matches("&#x2028;", "^.$"),
matches("&#x2029;", "^.$"),
matches("&#xA;", "^.$"),
matches("&#xD;", "^.$"),
matches("&#xD;&#xA;", "^.$", "s"),
matches("&#xA;", "^.$", "s"),
matches("&#xA0;", "^\s*$"),
matches("&#x85;", "\s"),
matches("&#x2028;", "^\s$"),
matches(" &#x9;&#xA;&#xD;", "^\s+$"),
matches("&#xA0;", "^\S$"),
matches("&#xA0;", "^[\s]$"),
matches("&#xA0;", "^[^\s]$"),
matches("a b", "^a[\sx]b$"),
replace("a&#x85;b", ".", "x")
