(: Under a collation that ignores case, each probe of an index with
   xs:anyURI or untyped keys selects what the comparison selects without an
   index. :)
import module namespace m = "http://www.example.com/collation-untyped-anyuri" at
                             "probe_untyped_anyuri.xqlib";

m:init();

fn:string-join((
  fn:concat("unindexed eq: ", m:value-unindexed(xs:anyURI("X"))),
  fn:concat("value hash: ", m:value-point("m:UriHash", xs:anyURI("X"))),
  fn:concat("value tree: ", m:value-point("m:UriTree", xs:anyURI("X"))),
  fn:concat("value tree range: ", m:value-range(xs:anyURI("X"))),
  for $key in (xs:anyURI("X"), xs:untypedAtomic("X"), "X")
  let $type := typeswitch ($key)
               case xs:anyURI return "anyURI"
               case xs:untypedAtomic return "untyped"
               default return "string"
  return (
    fn:concat("unindexed = ", $type, ": ", m:general-unindexed($key)),
    fn:concat("general hash ", $type, ": ", m:general-point("m:GeneralHash", $key)),
    fn:concat("general tree ", $type, ": ", m:general-point("m:GeneralTree", $key)))),
  "&#10;")
