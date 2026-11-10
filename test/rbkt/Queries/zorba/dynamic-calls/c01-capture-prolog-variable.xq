(: An inline function captures a prolog variable; it is called in its scope
   and from a function it is passed to. :)
declare variable $prefix := "v";
declare function local:call($f) { $f(1) };
let $f := function($i) { concat($prefix, $i) }
return ($f(1), local:call($f))
