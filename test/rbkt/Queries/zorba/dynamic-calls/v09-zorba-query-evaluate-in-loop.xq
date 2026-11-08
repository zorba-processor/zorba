(: zq:evaluate runs a prepared query that inserts a node through a function item: the call runs once per tuple, and the count of the nodes beside it reads them per tuple. :)
import module namespace zq = "http://zorba.io/modules/zorba-query";
declare variable $d := <a/>;
variable $key := zq:prepare-main-module("
  declare namespace an = 'http://zorba.io/annotations';
  declare variable $d external;
  declare %an:sequential function local:add() { insert node <b/> into $d; count($d/b) };
  declare variable $f := local:add#0;
  $f()");
zq:bind-variable($key, xs:QName("d"), $d);
for $i in 1 to 3 return (zq:evaluate($key), count($d/b))
