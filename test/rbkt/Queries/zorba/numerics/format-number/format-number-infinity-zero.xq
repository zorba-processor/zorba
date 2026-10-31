(: An adjusted number that overflows is infinity; negative zero uses the
   negative sub-picture. :)
string-join((
  fn:format-number(1e308, "0%"),
  fn:format-number(-1e308, "0%"),
  fn:format-number(0 div xs:double("-INF"), "#")
), "|") eq "Infinity%|-Infinity%|-0"
