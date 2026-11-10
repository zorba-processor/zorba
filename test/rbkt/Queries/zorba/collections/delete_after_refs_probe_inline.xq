(: Temporary probe, removed with the fix. :)
import module namespace m = "http://www.example.com/delete-after-local-refs" at
                             "delete_after_local_refs.xqlib";

import module namespace ddl =
  "http://zorba.io/modules/store/static/collections/ddl";
import module namespace dml =
  "http://zorba.io/modules/store/static/collections/dml";

declare namespace an = "http://zorba.io/annotations";
declare namespace err = "http://www.w3.org/2005/xqt-errors";

ddl:create(xs:QName("m:c"));
variable $g := %an:sequential function() as xs:string
{
  dml:insert(xs:QName("m:c"), <e v="1"/>);
  variable $nodes := dml:collection(xs:QName("m:c"));
  fn:string($nodes[1]/@v)
};
variable $inline := $g();
variable $inlineOutcome :=
  try {{ ddl:delete(xs:QName("m:c")); "deleted" }}
  catch * {{ m:clear(); fn:local-name-from-QName($err:code) }};

ddl:create(xs:QName("m:c"));
variable $prefix := "v";
variable $h := %an:sequential function() as xs:string
{
  dml:insert(xs:QName("m:c"), <e v="1"/>);
  variable $nodes := dml:collection(xs:QName("m:c"));
  fn:concat($prefix, fn:string($nodes[1]/@v))
};
variable $closure := $h();
variable $closureOutcome :=
  try {{ ddl:delete(xs:QName("m:c")); "deleted" }}
  catch * {{ m:clear(); fn:local-name-from-QName($err:code) }};

fn:string-join((
  fn:concat("inline function item: ", $inline, " ", $inlineOutcome),
  fn:concat("inline function item with a captured variable: ", $closure, " ",
            $closureOutcome)),
  "&#10;")
