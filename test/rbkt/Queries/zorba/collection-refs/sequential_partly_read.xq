(: A node bound to a local variable of a sequential function whose caller reads
   only its first item is no longer referenced once that statement has
   completed: ddl:delete does not raise ZDDY0015. :)
import module namespace m = "http://www.example.com/collection-refs" at
                             "refs.xqlib";

m:fill();
variable $head := fn:head(m:two());
variable $headOutcome := m:delete();

m:fill();
variable $exists := fn:exists(m:two());
variable $existsOutcome := m:delete();

fn:string-join((
  fn:concat("head: ", $head, " ", $headOutcome),
  fn:concat("exists: ", $exists, " ", $existsOutcome)),
  "&#10;")
