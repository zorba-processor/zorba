import module namespace m = "http://www.example.com/general-index-probe-gaps" at
                             "general_index_probe_gaps.xqlib";

import module namespace dml =
  "http://zorba.io/modules/store/static/collections/dml";

(: A general range index on the keys does not change what a WHERE clause
   selects with keys of types the index cannot probe. :)
m:create(xs:QName("m:h"),
         (<e id="1" v="2001"/>, <e id="2" v="1234"/>),
         xs:QName("m:HRange"));

fn:string-join((
  fn:concat("hexBinary: ",
    m:ids(for $e in dml:collection(xs:QName("m:h"))//e
          where $e/@v = xs:hexBinary("1234")
          return $e)),
  fn:concat("base64Binary: ",
    m:ids(for $e in dml:collection(xs:QName("m:h"))//e
          where $e/@v = xs:base64Binary("1234")
          return $e)),
  fn:concat("gYear: ",
    m:ids(for $e in dml:collection(xs:QName("m:h"))//e
          where $e/@v = xs:gYear("1234")
          return $e))),
  "&#10;")
