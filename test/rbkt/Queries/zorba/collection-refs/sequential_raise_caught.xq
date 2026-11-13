(: A node bound to a local variable of a sequential function that raised an
   error is no longer referenced once the caller caught the error: ddl:delete
   does not raise ZDDY0015. :)
import module namespace m = "http://www.example.com/collection-refs" at
                             "refs.xqlib";
import module namespace dml =
  "http://zorba.io/modules/store/static/collections/dml";

declare namespace an = "http://zorba.io/annotations";
declare namespace err = "http://www.w3.org/2005/xqt-errors";

(: Binds the nodes of m:c and calls m:raise, which raises. :)
declare %an:sequential function local:raise-through() as xs:string
{
  variable $nodes := dml:collection(xs:QName("m:c"));
  fn:concat(fn:string($nodes[1]/@v), m:raise())
};

declare %an:sequential function local:from-function() as xs:string
{
  m:fill();
  variable $caught :=
    try { m:raise() }
    catch * { fn:local-name-from-QName($err:code) };
  variable $outcome := m:delete();
  fn:concat("function: ", $caught, " ", $outcome)
};

m:fill();
variable $caught :=
  try { m:raise() }
  catch * { fn:concat(fn:local-name-from-QName($err:code), " ",
                      $err:description) };
variable $outcome := m:delete();

m:fill();
variable $through :=
  try { local:raise-through() }
  catch * { fn:local-name-from-QName($err:code) };
variable $throughOutcome := m:delete();

variable $fromFunction := local:from-function();

fn:string-join((
  fn:concat("main: ", $caught, " ", $outcome),
  fn:concat("main, through a caller: ", $through, " ", $throughOutcome),
  $fromFunction),
  "&#10;")
