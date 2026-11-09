import module namespace m = "http://www.example.com/general-numeric-promotion" at
                             "general_numeric_promotion.xqlib";

declare function local:range($name as xs:string,
                             $lower as xs:anyAtomicType?,
                             $upper as xs:anyAtomicType?,
                             $inclusive as xs:boolean) as xs:string
{
  m:range($name, xs:QName("m:c"), xs:QName("m:Range"), $lower, $upper,
          $inclusive)
};

m:init();

fn:string-join((
  local:range("decimal upper", (), 0.1, true()),
  local:range("exclusive decimal lower", 0.1, (), false()),
  local:range("exclusive integer upper", (), 16777217, false()),
  local:range("float upper", (), xs:float("16777216"), true()),
  local:range("float lower", xs:float("1e11"), (), true()),
  local:range("double lower", 1e11, (), true()),
  local:range("exclusive decimal upper on untyped integer", (),
              3.00000000000000000001, false()),
  local:range("untyped upper", (), xs:untypedAtomic("05"), true()),
  local:range("untyped lower", xs:untypedAtomic("16777217"), (), true()),
  local:range("no bounds", (), (), true())),
  "&#10;")
