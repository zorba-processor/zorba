jsoniq version "1.0";
(: Uses of the values other than lookups of literal keys get the values whole. :)
declare variable $log := "{\"a\":{\"b\":1,\"c\":2},\"k\":\"a\",\"n\":[1,2]}\n{\"a\":{\"b\":3},\"k\":\"n\",\"n\":[]}";
declare variable $options := { "jsoniq-multiple-top-level-items" : true };
declare function local:f($o) { keys($o) };
(
  [ for $v in parse-json($log, $options) return $v ],
  [ for $v in parse-json($log, $options) return keys($v) ],
  [ for $v in parse-json($log, $options) return $v($v.k) ],
  [ for $v in parse-json($log, $options) let $w := $v return $w.a ],
  [ for $v in parse-json($log, $options) return local:f($v.a) ],
  [ for $v in parse-json($log, $options) return local:f($v) ],
  [ for $v in parse-json($log, $options) group by $b := $v.a.b return count($v) ],
  [ for $v in parse-json($log, $options) return size($v.n) ],
  [ for $v in parse-json($log, $options) return $v.n[] ],
  [ for $v in parse-json($log, $options) return $v.a.c ],
  [ for $v in parse-json($log, $options) return serialize($v.a) ],
  [ for $v in parse-json($log, $options), $w in parse-json($log, $options)
    where $v.a.b eq $w.a.b return [ $v.k, $w.n ] ]
)
