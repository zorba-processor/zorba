(: A node bound to a local variable of a sequential function is no longer
   referenced once the function has returned: ddl:delete of its collection
   in the caller does not raise ZDDY0015. :)
import module namespace m = "http://www.example.com/delete-after-local-refs" at
                             "delete_after_local_refs.xqlib";

import module namespace ddl =
  "http://zorba.io/modules/store/static/collections/ddl";
import module namespace dml =
  "http://zorba.io/modules/store/static/collections/dml";

declare namespace an = "http://zorba.io/annotations";
declare namespace err = "http://www.w3.org/2005/xqt-errors";

ddl:create(xs:QName("m:c"));
variable $declared := m:declared();
variable $declaredOutcome :=
  try {{ ddl:delete(xs:QName("m:c")); "deleted" }}
  catch * {{ m:clear(); fn:local-name-from-QName($err:code) }};

ddl:create(xs:QName("m:c"));
variable $assigned := m:assigned();
variable $assignedOutcome :=
  try {{ ddl:delete(xs:QName("m:c")); "deleted" }}
  catch * {{ m:clear(); fn:local-name-from-QName($err:code) }};

ddl:create(xs:QName("m:c"));
variable $let := m:let();
variable $letOutcome :=
  try {{ ddl:delete(xs:QName("m:c")); "deleted" }}
  catch * {{ m:clear(); fn:local-name-from-QName($err:code) }};

ddl:create(xs:QName("m:c"));
variable $for := m:for();
variable $forOutcome :=
  try {{ ddl:delete(xs:QName("m:c")); "deleted" }}
  catch * {{ m:clear(); fn:local-name-from-QName($err:code) }};

ddl:create(xs:QName("m:c"));
variable $i := 0;
while ($i lt 2)
{
  m:declared();
  $i := $i + 1;
}
variable $loopOutcome :=
  try {{ ddl:delete(xs:QName("m:c")); "deleted" }}
  catch * {{ m:clear(); fn:local-name-from-QName($err:code) }};

ddl:create(xs:QName("m:c"));
$i := 0;
while ($i lt 2)
{
  m:assigned();
  $i := $i + 1;
}
variable $assignedLoopOutcome :=
  try {{ ddl:delete(xs:QName("m:c")); "deleted" }}
  catch * {{ m:clear(); fn:local-name-from-QName($err:code) }};

ddl:create(xs:QName("m:c"));
variable $f := m:declared#0;
variable $dynamic := $f();
variable $dynamicOutcome :=
  try {{ ddl:delete(xs:QName("m:c")); "deleted" }}
  catch * {{ m:clear(); fn:local-name-from-QName($err:code) }};

ddl:create(xs:QName("m:c"));
variable $inline := %an:sequential function() as xs:string
{
  dml:insert(xs:QName("m:c"), <e v="1"/>);
  variable $nodes := dml:collection(xs:QName("m:c"));
  fn:string($nodes[1]/@v)
};
variable $inlined := $inline();
variable $inlineOutcome :=
  try {{ ddl:delete(xs:QName("m:c")); "deleted" }}
  catch * {{ m:clear(); fn:local-name-from-QName($err:code) }};

ddl:create(xs:QName("m:c"));
variable $k := 1;
variable $closure := %an:sequential function() as xs:string
{
  dml:insert(xs:QName("m:c"), <e v="1"/>);
  variable $nodes := dml:collection(xs:QName("m:c"));
  fn:concat(fn:string($nodes[1]/@v), $k)
};
variable $captured := $closure();
variable $closureOutcome :=
  try {{ ddl:delete(xs:QName("m:c")); "deleted" }}
  catch * {{ m:clear(); fn:local-name-from-QName($err:code) }};

variable $fromFunction := m:from-function();

fn:string-join((
  fn:concat("main, declared variable: ", $declared, " ", $declaredOutcome),
  fn:concat("main, assigned variable: ", $assigned, " ", $assignedOutcome),
  fn:concat("main, let: ", $let, " ", $letOutcome),
  fn:concat("main, for: ", $for, " ", $forOutcome),
  fn:concat("main, declared variable, called in a loop: ", $loopOutcome),
  fn:concat("main, assigned variable, called in a loop: ",
            $assignedLoopOutcome),
  fn:concat("main, declared variable, function item: ", $dynamic, " ",
            $dynamicOutcome),
  fn:concat("main, inline function item: ", $inlined, " ", $inlineOutcome),
  fn:concat("main, inline function item with a captured variable: ",
            $captured, " ", $closureOutcome),
  $fromFunction),
  "&#10;")
