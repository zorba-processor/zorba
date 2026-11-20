(: In an index whose key type is xs:float, a decimal search key is promoted to
   xs:float, as in a comparison without an index. :)
import module namespace m = "http://www.example.com/index-keytype-parity" at
                             "keytype.xqlib";

import module namespace idml =
  "http://zorba.io/modules/store/static/indexes/dml";

(: External, so that the compiler does not see the type of each key. :)
declare variable $keys as xs:anyAtomicType* external := (0.1, 1);

m:create(xs:QName("m:f"), (<e id="1" v="0.1"/>, <e id="2" v="0.5"/>),
         (xs:QName("m:FloatHash"), xs:QName("m:FloatTree"),
          xs:QName("m:FloatValue")));

fn:string-join(
  for $key in $keys
  let $type := m:type($key)
  return (
    m:line(fn:concat("unindexed eq ", $type), function() {
      m:elements(xs:QName("m:f"))[xs:float(@v) eq $key] }),
    m:line(fn:concat("general hash value ", $type), function() {
      idml:probe-index-point-value(xs:QName("m:FloatHash"), $key) }),
    m:line(fn:concat("general tree value ", $type), function() {
      idml:probe-index-point-value(xs:QName("m:FloatTree"), $key) }),
    m:line(fn:concat("value tree ", $type), function() {
      idml:probe-index-point-value(xs:QName("m:FloatValue"), $key) }),
    m:line(fn:concat("unindexed = ", $type), function() {
      m:elements(xs:QName("m:f"))[xs:float(@v) = $key] }),
    m:line(fn:concat("general hash general ", $type), function() {
      idml:probe-index-point-general(xs:QName("m:FloatHash"), $key) }),
    m:line(fn:concat("general tree general ", $type), function() {
      idml:probe-index-point-general(xs:QName("m:FloatTree"), $key) }),
    m:line(fn:concat("unindexed range ", $type), function() {
      m:elements(xs:QName("m:f"))[xs:float(@v) ge $key and
                                  xs:float(@v) le $key] }),
    m:line(fn:concat("general tree range ", $type), function() {
      idml:probe-index-range-value(xs:QName("m:FloatTree"), $key, $key,
                                   fn:true(), fn:true(),
                                   fn:true(), fn:true()) }),
    m:line(fn:concat("value tree range ", $type), function() {
      idml:probe-index-range-value(xs:QName("m:FloatValue"), $key, $key,
                                   fn:true(), fn:true(),
                                   fn:true(), fn:true()) })),
  "&#10;")
