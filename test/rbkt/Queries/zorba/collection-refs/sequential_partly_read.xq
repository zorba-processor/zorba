(: A node bound to a local variable of a sequential function whose caller reads
   only its first item is no longer referenced once that statement has
   completed: ddl:delete does not raise ZDDY0015. :)
import module namespace m = "http://www.example.com/collection-refs" at
                             "refs.xqlib";

declare namespace an = "http://zorba.io/annotations";

declare %an:sequential function local:first() as xs:string*
{
  m:fill();
  variable $first := m:two()[1];
  variable $firstOutcome := m:delete();
  fn:concat("function, first item: ", $first, " ", $firstOutcome)
};

m:fill();
variable $head := fn:head(m:two());
variable $headOutcome := m:delete();

m:fill();
variable $exists := fn:exists(m:two());
variable $existsOutcome := m:delete();

m:fill();
variable $first := m:two()[1];
variable $firstOutcome := m:delete();

variable $fromFunction := local:first();

fn:string-join((
  fn:concat("head: ", $head, " ", $headOutcome),
  fn:concat("exists: ", $exists, " ", $existsOutcome),
  fn:concat("first item: ", $first, " ", $firstOutcome),
  $fromFunction),
  "&#10;")
