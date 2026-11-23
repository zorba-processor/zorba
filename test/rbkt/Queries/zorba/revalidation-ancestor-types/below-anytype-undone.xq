import schema namespace t = "http://zorba.io/test/revalidation-ancestor-types" at "ancestor-types.xsd";
declare namespace err = "http://www.w3.org/2005/xqt-errors";
declare revalidation skip;

(: Inserting an attribute that w already has fails after all the insertions
   were applied (XUDY0021), so they are undone: r gets its type back, past w
   of type xs:anyType, and has no typed value again. :)
variable $d := validate { document { <t:r><t:w k="0">x</t:w></t:r> } };
variable $code := "no error";
try
{
  (insert node <y/> into $d/t:r/t:w,
   insert node attribute k { 1 } into $d/t:r/t:w);
}
catch *
{
  $code := local-name-from-QName($err:code);
}
(
  $code,
  serialize($d/t:r),
  try { data($d/t:r) } catch * { local-name-from-QName($err:code) }
)
