(: In an index whose key type is xs:untypedAtomic, a general probe with a
   numeric search key casts the untyped keys to xs:double, as = does without
   an index; a value probe raises the error that eq raises. :)
import module namespace m = "http://www.example.com/index-keytype-parity" at
                             "keytype.xqlib";

import module namespace idml =
  "http://zorba.io/modules/store/static/indexes/dml";

(: External, so that the compiler does not see the type of each key. :)
declare variable $keys as xs:anyAtomicType* external := (2.5, 1, "1");

m:create(xs:QName("m:u"), (<e id="1" v="1"/>, <e id="2" v="2.5"/>),
         xs:QName("m:UntypedHash"));

fn:string-join(
  for $key in $keys
  let $type := m:type($key)
  return (
    m:line(fn:concat("unindexed eq ", $type), function() {
      m:elements(xs:QName("m:u"))[fn:data(@v) eq $key] }),
    m:line(fn:concat("hash value ", $type), function() {
      idml:probe-index-point-value(xs:QName("m:UntypedHash"), $key) }),
    m:line(fn:concat("unindexed = ", $type), function() {
      m:elements(xs:QName("m:u"))[fn:data(@v) = $key] }),
    m:line(fn:concat("hash general ", $type), function() {
      idml:probe-index-point-general(xs:QName("m:UntypedHash"), $key) })),
  "&#10;")
