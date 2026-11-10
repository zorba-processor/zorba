import module namespace m = "http://www.example.com/general-index-probe-gaps" at
                             "general_index_probe_gaps.xqlib";

import module namespace idml =
  "http://zorba.io/modules/store/static/indexes/dml";

declare namespace err = "http://www.w3.org/2005/xqt-errors";

m:create(xs:QName("m:s"),
         (<e id="1" t="dt" v="2013-01-13T12:01:53+01:00"/>,
          <e id="2" t="dts" v="2013-01-13T12:01:53+01:00"/>,
          <e id="3" t="dts" v="2014-01-13T12:01:53+01:00"/>,
          <e id="4" t="n" v=""/>,
          <e id="5" t="dt" v="2015-01-13T12:01:53Z"/>),
         (xs:QName("m:SEquality"), xs:QName("m:SRange")));

(: xs:dateTime and xs:dateTimeStamp values compare as xs:dateTime values;
   jn:null() equals only jn:null() and is less than every other value. :)
fn:string-join((
  for $index in ("SEquality", "SRange")
  for $probe in (
    ["dateTime", xs:dateTime("2013-01-13T12:01:53+01:00")],
    ["dateTimeStamp", xs:dateTimeStamp("2013-01-13T12:01:53+01:00")],
    ["dateTimeStamp, no key", xs:dateTimeStamp("2016-01-13T12:01:53Z")],
    ["null", jn:null()])
  return m:point(fn:concat($index, ", ", $probe(1)), xs:QName("m:s"),
                 xs:QName(fn:concat("m:", $index)), $probe(2)),
  for $bound in (
    ["dateTimeStamp", xs:dateTimeStamp("2014-01-13T12:01:53+01:00")],
    ["dateTime", xs:dateTime("2014-01-13T12:01:53+01:00")],
    ["null", jn:null()])
  for $inclusive in (fn:true(), fn:false())
  let $name := fn:concat($bound(1), if ($inclusive) then "" else " exclusive")
  return (
    m:range(fn:concat("lower ", $name), xs:QName("m:s"), xs:QName("m:SRange"),
            $bound(2), (), $inclusive),
    m:range(fn:concat("upper ", $name), xs:QName("m:s"), xs:QName("m:SRange"),
            (), $bound(2), $inclusive)),
  m:range-value("dateTime to dateTimeStamp", xs:QName("m:s"),
                xs:QName("m:SRange"), xs:dateTime("2013-01-13T12:01:53+01:00"),
                xs:dateTimeStamp("2014-01-13T12:01:53+01:00"), fn:true(),
                fn:true()),
  m:range-value("null to null", xs:QName("m:s"), xs:QName("m:SRange"),
                jn:null(), jn:null(), fn:true(), fn:true()),
  (: A value range probe rejects a null bound with a bound of another type. :)
  fn:concat("null to dateTimeStamp: ",
            try
            {
              fn:count(idml:probe-index-range-value(xs:QName("m:SRange"),
                jn:null(), xs:dateTimeStamp("2014-01-13T12:01:53+01:00"),
                fn:true(), fn:true(), fn:true(), fn:true()))
            }
            catch *
            {
              fn:local-name-from-QName($err:code)
            })),
  "&#10;")
