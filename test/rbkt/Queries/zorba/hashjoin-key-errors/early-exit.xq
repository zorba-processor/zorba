(: fn:exists() stops at the first node the join returns, so the inner key of
   a later node, which casts "abc" to xs:integer, is not evaluated, with or
   without an index; a join evaluated once per document uses the nodes of
   that document, also when its key raises for a later node. :)
declare namespace err = "http://www.w3.org/2005/xqt-errors";

let $doc := <d><k>1</k><k>1</k><x>1</x><x>abc</x></d>
return fn:string-join((
  fn:concat("=: ",
    try {
      fn:string-join(
        for $a in $doc/k
        return fn:string(fn:exists(for $b in $doc/x where xs:integer($b) = xs:integer($a) return $b)), " ")
    } catch * { fn:local-name-from-QName($err:code) }),
  fn:concat("eq: ",
    try {
      fn:string-join(
        for $a in $doc/k
        return fn:string(fn:exists(for $b in $doc/x where xs:integer($b) eq xs:integer($a) return $b)), " ")
    } catch * { fn:local-name-from-QName($err:code) }),
  fn:concat("=, one join per document: ",
    try {
      fn:string-join(
        for $d in (<d><k>1</k><x>1</x></d>, <d><k>2</k><x>2</x><x>abc</x></d>)
        return
          for $a in $d/k
          return fn:string(fn:exists(for $b in $d/x where xs:integer($b) = xs:integer($a) return $b)), " ")
    } catch * { fn:local-name-from-QName($err:code) })),
  "&#10;")
