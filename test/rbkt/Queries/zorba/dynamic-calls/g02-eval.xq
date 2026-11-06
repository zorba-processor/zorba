(: A function item that eval returns, reaching a sequential function not named elsewhere. :)
import module namespace r = "http://zorba.io/modules/reflection";
declare namespace an = "http://zorba.io/annotations";
declare %an:assignable variable $g := 0;
declare %an:sequential function local:bump() { $g := $g + 1; $g };
declare variable $e := r:eval("local:bump#0");
for $i in 1 to 2 return $e()
