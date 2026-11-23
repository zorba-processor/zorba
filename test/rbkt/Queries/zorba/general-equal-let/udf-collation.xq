(: General = against the parameter of a function that is not inlined, under a
   default collation that is not the codepoint one: strings equal under it
   but not byte for byte are found. :)
declare default collation "http://zorba.io/collations/SECONDARY/en/EN";
declare namespace an = "http://zorba.io/annotations";

declare %an:nondeterministic function local:keys()
{
  ("Apple", "pear", "PLUM")
};

declare function local:in($p, $keys, $depth)
{
  if ($depth gt 0) then local:in($p, $keys, $depth - 1) else $p = $keys
};

string-join(
  let $keys := local:keys()
  for $p in ("apple", "PEAR", "plum", "fig", "APPLE")
  return string(local:in($p, $keys, 1)),
  " ")
