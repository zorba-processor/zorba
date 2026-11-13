import module namespace m = "http://www.example.com/general-index-contract" at
                             "general_index_contract.xqlib";

import module namespace idml =
  "http://zorba.io/modules/store/static/indexes/dml";

m:create(xs:QName("m:i"),
         (<e id="1" v="5"/>, <e id="2" v="1"/>, <e id="3" v="4"/>,
          <e id="4" v="2"/>, <e id="5" v="3"/>, <e id="6" v="7"/>,
          <e id="7" v="7"/>, <e id="8" v="7"/>),
         (xs:QName("m:IRange"), xs:QName("m:IEquality")));

(: The skip functions skip the first results of the probe of a general index
   as they do for a value index. :)
fn:string-join((
  for $skip in (0, 2, 5, 9)
  let $range := idml:probe-index-range-value-skip(xs:QName("m:IRange"), $skip,
                                                  1, 5,
                                                  fn:true(), fn:true(),
                                                  fn:true(), fn:true())
  return fn:concat("range skip ", $skip, ": ", fn:count($range),
                   " [", m:ids($range), "]"),
  for $skip in (0, 1, 3)
  let $point := idml:probe-index-point-value-skip(xs:QName("m:IEquality"),
                                                  $skip, 7)
  return fn:concat("point skip ", $skip, ": ", fn:count($point))),
  "&#10;")
