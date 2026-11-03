(: A return clause with side effects sees the values it assigned, with the domain read eagerly. :)
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
for sliding window $w in (1, 2, 3)
  start $s when true
  only end $e when $e - $s eq 1
return { $g := $g + count($w); $g }
