(: A sequential function whose parameter is a global variable that the body
   assigns before it compares the parameter: the parameter keeps the value
   the variable had at the call. :)
declare namespace an = "http://zorba.io/annotations";

declare variable $global := ("a", "b");

declare %an:sequential function local:in($p, $set, $depth)
{
  if ($depth gt 0)
  then local:in($p, $set, $depth - 1)
  else
  {
    $global := ("x", $p);
    concat(string($p = $set), "/", string($set = $p), "/", string(count($set)))
  }
};

for $p in ("a", "b", "a", "x", "a")
return local:in($p, $global, 1)
