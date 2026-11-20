(: With copy-namespaces preserve, no-inherit, a constructor may place its
   content without a copy (the no-copy optimization at O1 and O2). Copying such
   a tree gives each element the in-scope namespaces a copy of the copied tree
   gives it. :)
declare copy-namespaces preserve, no-inherit;

declare function local:nest($n as xs:integer)
{
  if ($n eq 0) then <c xmlns:q="urn:q"/> else <a xmlns:p="urn:p">{ local:nest($n - 1) }</a>
};
declare function local:prefixes($t)
{
  for $e in $t/descendant-or-self::*
  return string-join(for $p in in-scope-prefixes($e) order by $p return $p, ",")
};

copy $c := local:nest(3) modify () return local:prefixes($c)
