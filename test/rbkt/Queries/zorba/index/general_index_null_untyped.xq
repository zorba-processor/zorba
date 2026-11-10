import module namespace m = "http://www.example.com/general-index-probe-gaps" at
                             "general_index_probe_gaps.xqlib";

import module namespace idml =
  "http://zorba.io/modules/store/static/indexes/dml";

m:create(xs:QName("m:su"), (<e id="1" v="a"/>, <e id="2" v="1"/>),
         xs:QName("m:SURange"));

(: A value comparison casts an untyped key to xs:string, which is equal to no
   null and greater than null. :)
fn:string-join((
  m:point-value("null value probe", xs:QName("m:su"), xs:QName("m:SURange"),
                jn:null()),
  for $inclusive in (fn:true(), fn:false())
  return m:result(
    fn:concat("null lower value bound",
              if ($inclusive) then "" else " exclusive"),
    m:ids(idml:probe-index-range-value(xs:QName("m:SURange"), jn:null(), (),
                                       fn:true(), fn:false(), $inclusive,
                                       fn:false())),
    m:ids(for $e in m:elements(xs:QName("m:su"))
          let $k := m:key($e)
          where (if ($inclusive) then $k ge jn:null() else $k gt jn:null())
          return $e))),
  "&#10;")
