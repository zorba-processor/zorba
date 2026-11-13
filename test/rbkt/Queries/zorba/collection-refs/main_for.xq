(: A node bound by a for or let in a completed statement of the main module is
   no longer referenced: ddl:delete in a later statement does not raise
   ZDDY0015. :)
import module namespace m = "http://www.example.com/collection-refs" at
                             "refs.xqlib";
import module namespace dml =
  "http://zorba.io/modules/store/static/collections/dml";

m:fill();
variable $for := fn:string-join(for $node in dml:collection(xs:QName("m:c"))
                                return fn:string($node/@v),
                                "");
variable $forOutcome := m:delete();

m:fill();
variable $let := let $nodes := dml:collection(xs:QName("m:c"))
                 return fn:concat(fn:count($nodes), fn:string($nodes[1]/@v));
variable $letOutcome := m:delete();

fn:string-join((
  fn:concat("for: ", $for, " ", $forOutcome),
  fn:concat("let: ", $let, " ", $letOutcome)),
  "&#10;")
