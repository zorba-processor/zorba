(: General = against a LET var under a default collation that is not the
   codepoint one: strings equal under it but not byte for byte are found. :)
declare default collation "http://zorba.io/collations/SECONDARY/en/EN";
declare namespace an = "http://zorba.io/annotations";

declare %an:nondeterministic function local:keys()
{
  ("Apple", "pear", "PLUM")
};

string-join(
  let $keys := local:keys()
  for $p in ("apple", "PEAR", "plum", "fig", "APPLE")
  return string($p = $keys),
  " ")
