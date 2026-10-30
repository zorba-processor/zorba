(: null is an item; a missing key is the empty sequence. :)
let $o := { "a": null, "b": [] }
return [
  count($o.a),
  count($o.missing),
  size($o.b),
  $o.a instance of null,
  empty($o.missing)
]
