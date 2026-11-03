(: F&O 3.0 op:numeric-equal, op:numeric-less-than on xs:unsignedLong above 2^63-1,
   a subtype of xs:integer: "Returns true if and only if the value of $arg1 is
   equal to the value of $arg2"; xs:decimal exactly, xs:double and xs:float after
   promotion. One row per left operand, one letter per right operand: l(ess),
   e(qual), g(reater), x if eq, ne, lt, le, gt, ge, =, <, > disagree. :)
declare namespace err = "http://www.w3.org/2005/xqt-errors";
let $v := (-1, xs:long("-1"), xs:nonNegativeInteger("5"), xs:long("9223372036854775807"), 9223372036854775807,
           xs:unsignedLong("9223372036854775807"), xs:unsignedLong("9223372036854775808"),
           9223372036854775808.0, xs:unsignedLong("18446744073709551615"), 18446744073709551615.0,
           18446744073709551616.0)
for $a in $v
return
  try {
    string-join(
      for $b in $v
      return
        if ($a eq $b and $a = $b and $a le $b and $a ge $b and
            not($a ne $b) and not($a lt $b) and not($a gt $b)) then "e"
        else if ($a lt $b and $a < $b and $a le $b and $a ne $b and
                 not($a eq $b) and not($a gt $b) and not($a ge $b)) then "l"
        else if ($a gt $b and $a > $b and $a ge $b and $a ne $b and
                 not($a eq $b) and not($a lt $b) and not($a le $b)) then "g"
        else "x",
      "")
  } catch * { local-name-from-QName($err:code) }
