(: XSD/F&O regular expression syntax that ICU reads differently (QT3
   fn-matches-50, Perl-derived tests). :)
declare namespace err = "http://www.w3.org/2005/xqt-errors";
string-join((
  (: invalid in XSD: a quantified quantifier, non-XSD property names :)
  for $r in ("a{1,5}+a", "a{2}{3}", "\p{L_}", "[^\p{Alphabetic}]", "\P{Cs}")
  return try { string(matches("aaaaa", $r)) }
         catch * { local-name-from-QName($err:code) },
  (: '{' and '}' are ordinary characters in a character class :)
  string(matches("ab1", "ab[(?{1]")),
  string(matches("ab}", "^ab[}]$")),
  (: the 'x' flag removes whitespace only; '#' is an ordinary character :)
  string(matches("a#b", "[#]", "x")),
  string(matches("a#b", "a # b", "x")),
  (: \NN is a back-reference only if there are NN groups before it :)
  string(matches("abcdefghija1", "^(a)(b)(c)(d)(e)(f)(g)(h)(i)(j)\11$")),
  string(matches("abcdefghijj", "^(a)(b)(c)(d)(e)(f)(g)(h)(i)(j)\10$")),
  string(matches("a", "\p{Lu}", "i"))
), " ")
