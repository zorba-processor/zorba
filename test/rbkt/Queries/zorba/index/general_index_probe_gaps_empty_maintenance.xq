import module namespace m = "http://www.example.com/general-index-probe-gaps" at
                             "general_index_probe_gaps.xqlib";

import module namespace ddl =
  "http://zorba.io/modules/store/static/collections/ddl";

import module namespace dml =
  "http://zorba.io/modules/store/static/collections/dml";

import module namespace iddl =
  "http://zorba.io/modules/store/static/indexes/ddl";

(: An automatic index keeps the nodes whose key is the empty sequence up to
   date, so a range probe without bounds selects every domain node. :)
ddl:create(xs:QName("m:ea"));
dml:insert(xs:QName("m:ea"), (<e id="1" v="a"/>, <e id="2"/>));
iddl:create(xs:QName("m:EaRange"));

dml:delete(dml:collection(xs:QName("m:ea"))[@id = "2"]);
dml:insert(xs:QName("m:ea"), <e id="3"/>);
variable $inserted :=
  m:no-bounds("after insert and delete", xs:QName("m:ea"),
              xs:QName("m:EaRange"));

insert node attribute v { "b" } into dml:collection(xs:QName("m:ea"))[@id = "3"];
delete node dml:collection(xs:QName("m:ea"))[@id = "1"]/@v;
variable $updated := (
  m:no-bounds("after updates", xs:QName("m:ea"), xs:QName("m:EaRange")),
  m:range("from a after updates", xs:QName("m:ea"), xs:QName("m:EaRange"),
          "a", (), fn:true()));

dml:truncate(xs:QName("m:ea"));
variable $truncated :=
  m:no-bounds("after truncate", xs:QName("m:ea"), xs:QName("m:EaRange"));

dml:insert(xs:QName("m:ea"), <e id="4"/>);

fn:string-join((
  $inserted,
  $updated,
  $truncated,
  m:no-bounds("after insert into the truncated collection", xs:QName("m:ea"),
              xs:QName("m:EaRange"))),
  "&#10;")
