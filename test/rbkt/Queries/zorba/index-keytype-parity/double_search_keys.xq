(: In an index whose key type is xs:double, each probe raises the error that
   its comparison raises without an index: an xs:string search key cannot be
   compared with an xs:double key. A decimal search key is promoted. :)
import module namespace m = "http://www.example.com/index-keytype-parity" at
                             "keytype.xqlib";

import module namespace idml =
  "http://zorba.io/modules/store/static/indexes/dml";

(: External, so that the compiler does not see the type of each key. :)
declare variable $keys as xs:anyAtomicType* external := ("1.5", 1.5);

m:create(xs:QName("m:d"), (<e id="1" v="1.5"/>, <e id="2" v="2"/>),
         (xs:QName("m:DoubleHash"), xs:QName("m:DoubleTree"),
          xs:QName("m:DoubleValue")));

fn:string-join(
  for $key in $keys
  let $type := m:type($key)
  return (
    m:line(fn:concat("unindexed eq ", $type), function() {
      m:elements(xs:QName("m:d"))[xs:double(@v) eq $key] }),
    m:line(fn:concat("general hash value ", $type), function() {
      idml:probe-index-point-value(xs:QName("m:DoubleHash"), $key) }),
    m:line(fn:concat("general tree value ", $type), function() {
      idml:probe-index-point-value(xs:QName("m:DoubleTree"), $key) }),
    m:line(fn:concat("value hash ", $type), function() {
      idml:probe-index-point-value(xs:QName("m:DoubleValue"), $key) }),
    m:line(fn:concat("unindexed = ", $type), function() {
      m:elements(xs:QName("m:d"))[xs:double(@v) = $key] }),
    m:line(fn:concat("general hash general ", $type), function() {
      idml:probe-index-point-general(xs:QName("m:DoubleHash"), $key) }),
    m:line(fn:concat("general tree general ", $type), function() {
      idml:probe-index-point-general(xs:QName("m:DoubleTree"), $key) }),
    m:line(fn:concat("unindexed range ", $type), function() {
      m:elements(xs:QName("m:d"))[xs:double(@v) ge $key and
                                  xs:double(@v) le $key] }),
    m:line(fn:concat("general tree range ", $type), function() {
      idml:probe-index-range-value(xs:QName("m:DoubleTree"), $key, $key,
                                   fn:true(), fn:true(),
                                   fn:true(), fn:true()) })),
  "&#10;")
