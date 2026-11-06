(: A function that is not sequential edits a node of a collection in the modify
   clause of a copy modify expression, and the loop reads that node after each
   call. The error is the same at every optimization level (this test also runs
   at O0 and O2). :)
import module namespace ddl = "http://zorba.io/modules/store/dynamic/collections/ddl";
import module namespace dml = "http://zorba.io/modules/store/dynamic/collections/dml";
declare function local:grow($k) {
  copy $c := <x/>
  modify dml:edit(dml:collection(xs:QName("local:c"))[1],
                  <a>{for $j in 1 to $k return <b/>}</a>)
  return $c[false()]
};
ddl:create(xs:QName("local:c"));
dml:insert-last(xs:QName("local:c"), <a/>);
for $n in dml:collection(xs:QName("local:c"))
return for $i in 1 to 3 return (local:grow($i), count($n/b))
