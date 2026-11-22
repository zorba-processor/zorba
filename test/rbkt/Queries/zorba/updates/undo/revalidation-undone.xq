import module namespace m = 'xqueryzorba.org/test/xqddf/ic' at '../../xqddf/ic.xqlib';
import module namespace ddl = "http://zorba.io/modules/store/static/collections/ddl";
import module namespace dml = "http://zorba.io/modules/store/static/collections/dml";
import module namespace ic_ddl = "http://zorba.io/modules/store/static/integrity-constraints/ddl";
import schema namespace s = "urn:revalidation-undone" at "revalidation-undone.xsd";
declare namespace err = "http://www.w3.org/2005/xqt-errors";

declare revalidation strict;

(: Renaming b to a makes revalidation give the element its type and the
   default attribute d. The integrity constraint fails after that, so the
   statement is undone: the element is b again, without d, with its type. :)
variable $d := validate { document { <s:r><s:b>02</s:b></s:r> } };

ddl:create($m:empc);
ic_ddl:activate(xs:QName("m:ic_simple"));

variable $code := "no error";
try
{
  (rename node $d/s:r/s:b as QName("urn:revalidation-undone", "a"),
   dml:insert($m:empc, <emp><salary>600</salary></emp>));
}
catch *
{
  $code := local-name-from-QName($err:code);
}
(
  $code,
  serialize($d/s:r),
  $d/s:r/s:b instance of element(s:b, xs:string),
  data($d/s:r/s:b) instance of xs:string
)
