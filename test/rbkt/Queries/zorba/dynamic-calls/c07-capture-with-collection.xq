(: A sequential inline function captures a variable of the main program and
   binds the nodes of a collection to a local variable. :)
import module namespace ddl = "http://zorba.io/modules/store/dynamic/collections/ddl";
import module namespace dml = "http://zorba.io/modules/store/dynamic/collections/dml";
declare namespace an = "http://zorba.io/annotations";
ddl:create(xs:QName("local:c"));
variable $prefix := "v";
variable $h := %an:sequential function() as xs:string
{
  dml:insert(xs:QName("local:c"), <e v="1"/>);
  variable $nodes := dml:collection(xs:QName("local:c"));
  concat($prefix, string($nodes[1]/@v))
};
variable $r := $h();
dml:delete(dml:collection(xs:QName("local:c")));
ddl:delete(xs:QName("local:c"));
$r
