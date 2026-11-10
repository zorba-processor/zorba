(: Under the codepoint collation, a predicate is not answered by an index
   declared under a collation that ignores case; values of xs:string and
   xs:anyURI that are equal hash alike. :)
import module namespace m = "http://www.example.com/index-collation-match" at
                             "match.xqlib";

import module namespace dml =
  "http://zorba.io/modules/store/static/collections/dml";

import module namespace idml =
  "http://zorba.io/modules/store/static/indexes/dml";

m:init();

fn:string-join((
  fn:concat("string, secondary index: ",
    m:names(dml:collection(xs:QName("m:secondary"))
              [xs:string(@email) eq "George@GMAIL.com"])),
  fn:concat("string, codepoint index: ",
    m:names(dml:collection(xs:QName("m:codepoint"))
              [xs:string(@email) eq "george@gmail.com"])),
  fn:concat("anyURI, secondary index: ",
    m:names(dml:collection(xs:QName("m:uris"))
              [xs:anyURI(@email) eq xs:anyURI("George@GMAIL.com")])),
  fn:concat("anyURI probe of a string index: ",
    m:names(idml:probe-index-point-value(xs:QName("m:by-email-codepoint"),
                                         xs:anyURI("george@gmail.com")))),
  fn:concat("distinct string and anyURI: ",
    fn:count(fn:distinct-values(("x", xs:anyURI("x"))))),
  fn:concat("group by string and anyURI: ",
    fn:string-join(for $k in ("x", xs:anyURI("x"))
                   group by $k
                   return fn:string(fn:count($k)), " "))),
  "&#10;")
