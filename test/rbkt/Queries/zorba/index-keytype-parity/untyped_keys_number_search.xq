(: A general probe with a numeric search key in an index with untyped keys
   raises the error that = raises without an index when an untyped key is not
   castable to xs:double. :)
import module namespace m = "http://www.example.com/index-keytype-parity" at
                             "keytype.xqlib";

import module namespace idml =
  "http://zorba.io/modules/store/static/indexes/dml";

(: External, so that the compiler does not see the type of each key. :)
declare variable $keys as xs:anyAtomicType* external := (1, "1");

m:create(xs:QName("m:ua"), (<e id="1" v="1"/>, <e id="2" v="a"/>),
         (xs:QName("m:AnyHash"), xs:QName("m:AnyTree"),
          xs:QName("m:UntypedKeysHash")));

fn:string-join(
  for $key in $keys
  let $type := m:type($key)
  return (
    m:line(fn:concat("unindexed = ", $type), function() {
      m:elements(xs:QName("m:ua"))[fn:data(@v) = $key] }),
    m:line(fn:concat("hash general ", $type), function() {
      idml:probe-index-point-general(xs:QName("m:AnyHash"), $key) }),
    m:line(fn:concat("tree general ", $type), function() {
      idml:probe-index-point-general(xs:QName("m:AnyTree"), $key) }),
    m:line(fn:concat("untyped hash general ", $type), function() {
      idml:probe-index-point-general(xs:QName("m:UntypedKeysHash"), $key) }),
    m:line(fn:concat("unindexed general range ", $type), function() {
      m:elements(xs:QName("m:ua"))[fn:data(@v) >= $key and
                                   fn:data(@v) <= $key] }),
    m:line(fn:concat("tree general range ", $type), function() {
      idml:probe-index-range-general(xs:QName("m:AnyTree"), $key, $key,
                                     fn:true(), fn:true(),
                                     fn:true(), fn:true()) })),
  "&#10;")
