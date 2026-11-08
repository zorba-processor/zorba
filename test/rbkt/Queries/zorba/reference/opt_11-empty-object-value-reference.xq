(: An object constructor evaluates a value although the type of the value is the empty sequence. :)
import module namespace ref = "http://zorba.io/modules/reference";
declare namespace an = "http://zorba.io/annotations";

(: Its result type is the empty sequence; it gives the node a reference. Recursive, so it is not inlined. :)
declare %an:nondeterministic function local:mark($n as node(), $k as xs:integer) as empty-sequence()
{
  if ($k gt 0)
  then local:mark($n, $k - 1)
  else if (string-length(ref:reference($n)) gt 0) then () else ()
};

let $n := <a/>
return ({ "a" : local:mark($n, 1) }("a") instance of js:null, ref:has-reference($n))
