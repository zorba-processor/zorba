(: A join under an empty outer domain evaluates neither its inner domain nor
   its inner key, with or without an index: the cast of "abc" to xs:integer,
   in the key or in the domain, is not evaluated. :)
declare namespace err = "http://www.w3.org/2005/xqt-errors";

let $doc := <d><x k="1">1</x><x k="2">abc</x></d>
return fn:string-join((
  fn:concat("=, raising inner key: ",
    try {
      fn:string(fn:count(
        for $a in $doc/none
        let $m := for $b in $doc/x where xs:integer($b) = xs:integer($a/@y) return $b
        return fn:count($m)))
    } catch * { fn:local-name-from-QName($err:code) }),
  fn:concat("eq, raising inner key: ",
    try {
      fn:string(fn:count(
        for $a in $doc/none
        let $m := for $b in $doc/x where xs:integer($b) eq xs:integer($a/@y) return $b
        return fn:count($m)))
    } catch * { fn:local-name-from-QName($err:code) }),
  fn:concat("=, raising inner domain: ",
    try {
      fn:string(fn:count(
        for $a in $doc/none
        let $m := for $b in $doc/x[xs:integer(.) gt 0] where fn:string($b/@k) = fn:string($a/@k) return $b
        return fn:count($m)))
    } catch * { fn:local-name-from-QName($err:code) }),
  fn:concat("eq, raising inner domain: ",
    try {
      fn:string(fn:count(
        for $a in $doc/none
        let $m := for $b in $doc/x[xs:integer(.) gt 0] where fn:string($b/@k) eq fn:string($a/@k) return $b
        return fn:count($m)))
    } catch * { fn:local-name-from-QName($err:code) })),
  "&#10;")
