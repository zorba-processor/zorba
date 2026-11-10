(: A conjunct before the join comparison decides for which pairs the
   comparison is evaluated, with or without an index: the inner key that casts
   "abc" to xs:integer is not evaluated for the node the guard rejects, and a
   guard that raises an error raises it for every pair it is evaluated for. :)
declare namespace err = "http://www.w3.org/2005/xqt-errors";

let $doc := <d><x t="int" k="1">1</x><x t="str" k="2">abc</x></d>
return fn:string-join((
  fn:concat("=, guarded inner key: ",
    try {
      fn:string-join(
        for $k in (1, 2)
        let $m := for $b in $doc/x where $b/@t = "int" and xs:integer($b) = $k return $b
        return fn:string(fn:count($m)), " ")
    } catch * { fn:local-name-from-QName($err:code) }),
  fn:concat("eq, guarded inner key: ",
    try {
      fn:string-join(
        for $k in (1, 2)
        let $m := for $b in $doc/x where $b/@t = "int" and xs:integer($b) eq $k return $b
        return fn:string(fn:count($m)), " ")
    } catch * { fn:local-name-from-QName($err:code) }),
  fn:concat("eq, guard in an earlier where clause: ",
    try {
      fn:string-join(
        for $k in (1, 2)
        let $m := for $b in $doc/x where $b/@t = "int" where xs:integer($b) eq $k return $b
        return fn:string(fn:count($m)), " ")
    } catch * { fn:local-name-from-QName($err:code) }),
  fn:concat("=, raising guard: ",
    try {
      fn:string-join(
        for $k in ("1", "3")
        let $m := for $b in $doc/x where xs:integer($b) gt 0 and fn:string($b/@k) = $k return $b
        return fn:string(fn:count($m)), " ")
    } catch * { fn:local-name-from-QName($err:code) })),
  "&#10;")
