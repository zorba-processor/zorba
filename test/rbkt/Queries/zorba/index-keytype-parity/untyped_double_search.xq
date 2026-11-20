(: In an index whose key type is xs:double, an untyped search key raises in a
   value probe the error that eq raises without an index, since it compares
   as an xs:string, and in a general probe is cast to xs:double, as = does. :)
import module namespace m = "http://www.example.com/index-keytype-parity" at
                             "keytype.xqlib";

import module namespace idml =
  "http://zorba.io/modules/store/static/indexes/dml";

(: External, so that the compiler does not see the type of each key. :)
declare variable $keys as xs:anyAtomicType* external :=
  (xs:untypedAtomic("1.5"), xs:untypedAtomic("a"));

m:create(xs:QName("m:d"), (<e id="1" v="1.5"/>, <e id="2" v="2"/>),
         (xs:QName("m:DoubleHash"), xs:QName("m:DoubleTree"),
          xs:QName("m:DoubleValue")));

fn:string-join(
  for $key in $keys
  return (
    m:line(fn:concat("unindexed eq ", $key), function() {
      m:elements(xs:QName("m:d"))[xs:double(@v) eq $key] }),
    m:line(fn:concat("general hash value ", $key), function() {
      idml:probe-index-point-value(xs:QName("m:DoubleHash"), $key) }),
    m:line(fn:concat("general tree value ", $key), function() {
      idml:probe-index-point-value(xs:QName("m:DoubleTree"), $key) }),
    m:line(fn:concat("value hash ", $key), function() {
      idml:probe-index-point-value(xs:QName("m:DoubleValue"), $key) }),
    m:line(fn:concat("unindexed = ", $key), function() {
      m:elements(xs:QName("m:d"))[xs:double(@v) = $key] }),
    m:line(fn:concat("general hash general ", $key), function() {
      idml:probe-index-point-general(xs:QName("m:DoubleHash"), $key) }),
    m:line(fn:concat("general tree general ", $key), function() {
      idml:probe-index-point-general(xs:QName("m:DoubleTree"), $key) }),
    m:line(fn:concat("unindexed range ", $key), function() {
      m:elements(xs:QName("m:d"))[xs:double(@v) ge $key and
                                  xs:double(@v) le $key] }),
    m:line(fn:concat("general tree range ", $key), function() {
      idml:probe-index-range-value(xs:QName("m:DoubleTree"), $key, $key,
                                   fn:true(), fn:true(),
                                   fn:true(), fn:true()) })),
  "&#10;")
