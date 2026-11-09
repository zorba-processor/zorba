import module namespace m = "http://www.example.com/general-index-probe-gaps" at
                             "general_index_probe_gaps.xqlib";

import module namespace ddl =
  "http://zorba.io/modules/store/static/collections/ddl";

import module namespace dml =
  "http://zorba.io/modules/store/static/collections/dml";

import module namespace iddl =
  "http://zorba.io/modules/store/static/indexes/ddl";

ddl:create(xs:QName("m:a"));
iddl:create(xs:QName("m:AEquality"));
iddl:create(xs:QName("m:ARange"));
dml:insert(xs:QName("m:a"),
           (<e id="1" t="d" v="NaN"/>, <e id="2" v="NaN"/>,
            <e id="3" t="d" v="1"/>, <e id="4" t="d" v="NaN"/>));

variable $inserted := m:keys("AEquality keys", xs:QName("m:AEquality"));

dml:delete(dml:collection(xs:QName("m:a"))[@id = ("1", "2", "4")]);

(: Two NaN keys are not equal, so they do not violate uniqueness. :)
ddl:create(xs:QName("m:u"));
variable $equalityNaN :=
  m:unique("UEquality NaN twice", xs:QName("m:UEquality"),
           <e id="1" t="d" v="NaN"/>, <e id="2" t="d" v="NaN"/>);
variable $rangeNaN :=
  m:unique("URange NaN twice", xs:QName("m:URange"),
           <e id="1" t="d" v="NaN"/>, <e id="2" t="d" v="NaN"/>);
variable $equalityOne :=
  m:unique("UEquality 1 twice", xs:QName("m:UEquality"),
           <e id="1" t="d" v="1"/>, <e id="2" t="d" v="1"/>);
variable $diagEq :=
  m:unique("UEquality 1 and 2", xs:QName("m:UEquality"),
           <e id="1" t="d" v="1"/>, <e id="2" t="d" v="2"/>);
variable $diagRange :=
  m:unique("URange 1 and 2", xs:QName("m:URange"),
           <e id="1" t="d" v="1"/>, <e id="2" t="d" v="2"/>);
variable $rangeOne :=
  m:unique("URange 1 twice", xs:QName("m:URange"),
           <e id="1" t="d" v="1"/>, <e id="2" t="d" v="1"/>);

fn:string-join((
  $inserted,
  m:keys("AEquality keys after delete", xs:QName("m:AEquality")),
  m:range("ARange no bounds after delete", xs:QName("m:a"),
          xs:QName("m:ARange"), (), ()),
  $equalityNaN,
  $rangeNaN,
  $equalityOne,
  $rangeOne,
  $diagEq,
  $diagRange),
  "&#10;")
