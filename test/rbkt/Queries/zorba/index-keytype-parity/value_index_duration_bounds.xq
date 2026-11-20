(: In a value range index whose key type is xs:duration, a range probe with
   duration bounds raises the error that ge and le raise without an index:
   only two xs:yearMonthDuration or two xs:dayTimeDuration values have an
   order, and xs:duration values have none. A point probe compares any two
   durations, as eq does. :)
import module namespace m = "http://www.example.com/index-keytype-parity" at
                             "keytype.xqlib";

import module namespace idml =
  "http://zorba.io/modules/store/static/indexes/dml";

(: External, so that the compiler does not see the type of each key. :)
declare variable $keys as xs:anyAtomicType* external :=
  (xs:yearMonthDuration("P1Y"), xs:dayTimeDuration("P1D"), xs:duration("P1Y"));

m:create(xs:QName("m:ym"), (<e id="1" v="P1Y"/>, <e id="2" v="P2Y"/>),
         (xs:QName("m:DurationValueTree"), xs:QName("m:YearMonthValueTree")));

fn:string-join(
  for $key in $keys
  let $type := m:type($key)
  return (
    m:line(fn:concat("unindexed duration eq ", $type), function() {
      m:elements(xs:QName("m:ym"))[xs:duration(@v) eq $key] }),
    m:line(fn:concat("duration value ", $type), function() {
      idml:probe-index-point-value(xs:QName("m:DurationValueTree"), $key) }),
    m:line(fn:concat("unindexed duration range ", $type), function() {
      m:elements(xs:QName("m:ym"))[xs:duration(@v) ge $key and
                                   xs:duration(@v) le $key] }),
    m:line(fn:concat("duration range ", $type), function() {
      idml:probe-index-range-value(xs:QName("m:DurationValueTree"), $key, $key,
                                   fn:true(), fn:true(),
                                   fn:true(), fn:true()) }),
    m:line(fn:concat("unindexed yearMonthDuration eq ", $type), function() {
      m:elements(xs:QName("m:ym"))[xs:yearMonthDuration(@v) eq $key] }),
    m:line(fn:concat("yearMonthDuration as duration value ", $type), function() {
      idml:probe-index-point-value(xs:QName("m:YearMonthValueTree"), $key) }),
    m:line(fn:concat("unindexed yearMonthDuration range ", $type), function() {
      m:elements(xs:QName("m:ym"))[xs:yearMonthDuration(@v) ge $key and
                                   xs:yearMonthDuration(@v) le $key] }),
    m:line(fn:concat("yearMonthDuration as duration range ", $type), function() {
      idml:probe-index-range-value(xs:QName("m:YearMonthValueTree"), $key, $key,
                                   fn:true(), fn:true(),
                                   fn:true(), fn:true()) })),
  "&#10;")
