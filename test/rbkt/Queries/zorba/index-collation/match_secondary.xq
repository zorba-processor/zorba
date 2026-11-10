(: Under a default collation that ignores case, a predicate is not answered
   by an index declared under the codepoint collation, and is answered by an
   index whose xs:string or xs:anyURI key takes the same default collation. :)
declare default collation "http://zorba.io/collations/SECONDARY/en/EN";

import module namespace m = "http://www.example.com/index-collation-match" at
                             "match.xqlib";

import module namespace dml =
  "http://zorba.io/modules/store/static/collections/dml";

m:init();

fn:string-join((
  fn:concat("string, secondary index: ",
    m:names(dml:collection(xs:QName("m:secondary"))
              [xs:string(@email) eq "George@GMAIL.com"])),
  fn:concat("string, codepoint index: ",
    m:names(dml:collection(xs:QName("m:codepoint"))
              [xs:string(@email) eq "George@GMAIL.com"])),
  fn:concat("anyURI, secondary index: ",
    m:names(dml:collection(xs:QName("m:uris"))
              [xs:anyURI(@email) eq xs:anyURI("George@GMAIL.com")]))),
  "&#10;")
