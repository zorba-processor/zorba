(: Temporary probe, removed with the fix. :)
import module namespace m = "http://www.example.com/delete-after-local-refs" at
                             "delete_after_local_refs.xqlib";

import module namespace ddl =
  "http://zorba.io/modules/store/static/collections/ddl";
import module namespace dml =
  "http://zorba.io/modules/store/static/collections/dml";

declare namespace an = "http://zorba.io/annotations";
declare namespace err = "http://www.w3.org/2005/xqt-errors";

declare function local:simple-for() as xs:string
{
  fn:string-join(for $node in dml:collection(xs:QName("m:c"))
                 return fn:string($node/@v),
                 "")
};

declare %an:sequential function local:raises() as xs:string
{
  dml:insert(xs:QName("m:c"), <e v="1"/>);
  variable $nodes := dml:collection(xs:QName("m:c"));
  fn:error(xs:QName("local:stop"), fn:string($nodes[1]/@v))
};

ddl:create(xs:QName("m:c"));
dml:insert(xs:QName("m:c"), <e v="1"/>);
variable $simple := local:simple-for();
variable $simpleOutcome :=
  try {{ ddl:delete(xs:QName("m:c")); "deleted" }}
  catch * {{ m:clear(); fn:local-name-from-QName($err:code) }};

ddl:create(xs:QName("m:c"));
dml:insert(xs:QName("m:c"), <e v="1"/>);
variable $mainFor := fn:string-join(for $node in dml:collection(xs:QName("m:c"))
                                    return fn:string($node/@v),
                                    "");
variable $mainForOutcome :=
  try {{ ddl:delete(xs:QName("m:c")); "deleted" }}
  catch * {{ m:clear(); fn:local-name-from-QName($err:code) }};

ddl:create(xs:QName("m:c"));
variable $raised := try {{ local:raises() }} catch * {{ "stopped" }};
variable $raisedOutcome :=
  try {{ ddl:delete(xs:QName("m:c")); "deleted" }}
  catch * {{ m:clear(); fn:local-name-from-QName($err:code) }};

fn:string-join((
  fn:concat("simple function, for: ", $simple, " ", $simpleOutcome),
  fn:concat("main, for: ", $mainFor, " ", $mainForOutcome),
  fn:concat("sequential function, error: ", $raised, " ", $raisedOutcome)),
  "&#10;")
