import schema namespace t = "http://zorba.io/test/revalidation-ancestor-types" at "ancestor-types.xsd";
declare namespace err = "http://www.w3.org/2005/xqt-errors";
declare revalidation skip;

(: An update below an element of type xs:anyType takes the types away from
   its ancestors too (upd:removeType): without revalidation, r is of type
   xs:anyType after it, so its typed value is its string value. :)
variable $d1 := validate { document { <t:r><t:w k="0">x</t:w></t:r> } };
variable $d2 := validate { document { <t:r><t:w k="0">x</t:w></t:r> } };
variable $d3 := validate { document { <t:r><t:w k="0">x<v/></t:w></t:r> } };
variable $before := try { data($d1/t:r) } catch * { local-name-from-QName($err:code) };
insert node <y>z</y> into $d1/t:r/t:w;
replace value of node $d2/t:r/t:w/text() with "u";
delete node $d3/t:r/t:w/*:v;
(
  $before,
  for $d in ($d1, $d2, $d3)
  return data($d/t:r) instance of xs:untypedAtomic,
  data($d1/t:r), data($d2/t:r), data($d3/t:r)
)
