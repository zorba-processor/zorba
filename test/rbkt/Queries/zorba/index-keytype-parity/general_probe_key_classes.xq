(: For each class of keys and each class of search key, a probe raises the
   error that the comparison it stands for raises without an index, and
   selects what it selects: = for the general point probes, >= and <= for the
   general range probes, eq for the value point probes, ge and le for the
   value range probes. Keys of one class compare only with search keys of the
   same class; = and >= cast an untyped search key to the type of the key, eq
   and ge cast it to xs:string. :)
import module namespace m = "http://www.example.com/index-keytype-parity" at
                             "keytype.xqlib";

import module namespace idml =
  "http://zorba.io/modules/store/static/indexes/dml";

(: External, so that the compiler does not see the type of each key. :)
declare variable $keys as xs:anyAtomicType* external :=
  (1, "1", xs:untypedAtomic("1"), xs:date("2020-01-01"),
   xs:yearMonthDuration("P1Y"), fn:true());

declare function local:lines($class as xs:string,
                             $collection as xs:QName,
                             $key as function(element(e)) as xs:anyAtomicType,
                             $hash as xs:QName?,
                             $tree as xs:QName) as xs:string*
{
  for $search in $keys
  let $label := fn:concat($class, " keys, ", m:type($search), " search key, ")
  let $elements := function() { m:elements($collection) }
  return (
    m:line(fn:concat($label, "unindexed ="), function() {
      $elements()[$key(.) = $search] }),
    if (fn:exists($hash))
    then m:line(fn:concat($label, "hash general"), function() {
           idml:probe-index-point-general($hash, $search) })
    else (),
    m:line(fn:concat($label, "tree general"), function() {
      idml:probe-index-point-general($tree, $search) }),
    m:line(fn:concat($label, "unindexed >= <="), function() {
      $elements()[$key(.) >= $search and $key(.) <= $search] }),
    m:line(fn:concat($label, "tree general range"), function() {
      idml:probe-index-range-general($tree, $search, $search,
                                     fn:true(), fn:true(),
                                     fn:true(), fn:true()) }),
    m:line(fn:concat($label, "unindexed eq"), function() {
      $elements()[$key(.) eq $search] }),
    if (fn:exists($hash))
    then m:line(fn:concat($label, "hash value"), function() {
           idml:probe-index-point-value($hash, $search) })
    else (),
    m:line(fn:concat($label, "unindexed ge le"), function() {
      $elements()[$key(.) ge $search and $key(.) le $search] }),
    m:line(fn:concat($label, "tree value range"), function() {
      idml:probe-index-range-value($tree, $search, $search,
                                   fn:true(), fn:true(),
                                   fn:true(), fn:true()) }))
};

m:create(xs:QName("m:ks"), <e id="1" v="1"/>,
         (xs:QName("m:StringKeysHash"), xs:QName("m:StringKeysTree")));
m:create(xs:QName("m:kn"), <e id="1" v="1"/>,
         (xs:QName("m:NumberKeysHash"), xs:QName("m:NumberKeysTree")));
m:create(xs:QName("m:kb"), <e id="1" v="true"/>,
         (xs:QName("m:BooleanKeysHash"), xs:QName("m:BooleanKeysTree")));
m:create(xs:QName("m:kd"), <e id="1" v="2020-01-01"/>,
         (xs:QName("m:DateKeysHash"), xs:QName("m:DateKeysTree")));
m:create(xs:QName("m:ky"), <e id="1" v="P1Y"/>,
         (xs:QName("m:YearMonthKeysHash"), xs:QName("m:YearMonthKeysTree"),
          xs:QName("m:YearMonthKeysTypedTree")));
m:create(xs:QName("m:ua"), (<e id="1" v="1"/>, <e id="2" v="a"/>),
         (xs:QName("m:AnyHash"), xs:QName("m:AnyTree")));

fn:string-join((
  local:lines("string", xs:QName("m:ks"), function($e) { fn:string($e/@v) },
              xs:QName("m:StringKeysHash"), xs:QName("m:StringKeysTree")),
  local:lines("number", xs:QName("m:kn"), function($e) { xs:integer($e/@v) },
              xs:QName("m:NumberKeysHash"), xs:QName("m:NumberKeysTree")),
  local:lines("boolean", xs:QName("m:kb"), function($e) { xs:boolean($e/@v) },
              xs:QName("m:BooleanKeysHash"), xs:QName("m:BooleanKeysTree")),
  local:lines("date", xs:QName("m:kd"), function($e) { xs:date($e/@v) },
              xs:QName("m:DateKeysHash"), xs:QName("m:DateKeysTree")),
  local:lines("yearMonthDuration", xs:QName("m:ky"),
              function($e) { xs:yearMonthDuration($e/@v) },
              xs:QName("m:YearMonthKeysHash"),
              xs:QName("m:YearMonthKeysTree")),
  local:lines("yearMonthDuration as duration", xs:QName("m:ky"),
              function($e) { xs:yearMonthDuration($e/@v) },
              (), xs:QName("m:YearMonthKeysTypedTree")),
  local:lines("untyped", xs:QName("m:ua"), function($e) { fn:data($e/@v) },
              xs:QName("m:AnyHash"), xs:QName("m:AnyTree"))),
  "&#10;")
