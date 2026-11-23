(: General = against the parameter of a function that is not inlined, whose
   value has nodes, objects, arrays or functions: the comparison atomizes them
   (fn:data()), with the result and the errors, at their locations, of the
   nested loop over the pairs. :)
declare namespace an = "http://zorba.io/annotations";
declare namespace err = "http://www.w3.org/2005/xqt-errors";

declare %an:nondeterministic function local:nodes()
{
  (<a>x</a>, <b>2</b>, "y", <c/>)
};

(: $p[1], not a var reference, leaves $set the hashed operand of $set = $p[1]. :)
declare function local:in($p, $set, $depth)
{
  if ($depth gt 0)
  then local:in($p, $set, $depth - 1)
  else try { string($p = $set) || "/" || string($set = $p[1]) }
       catch * { local-name-from-QName($err:code) || "@" || $err:line-number || ":" || $err:column-number }
};

declare function local:each($probes, $set)
{
  string-join(for $p in $probes return local:in($p, $set, 1), " ")
};

{ "nodes" :
    let $set := local:nodes()
    return local:each(("x", "y", "z", "", 2, "2", "x"), $set),
  "object" :
    let $set := (local:nodes()[3], { "a" : 1 })
    return local:each(("y", "z", "y"), $set),
  "array" :
    let $set := ("y", [ 1 ], local:nodes()[1])
    return local:each(("y", "x", "y", "z"), $set),
  "function" :
    let $set := (local:nodes()[3], concat#2)
    return local:each(("y", "z", "y"), $set),
  "node probes" :
    let $set := ("x", "2", "y")
    return local:each((<p>x</p>, <q>z</q>, <p>x</p>), $set)
}
