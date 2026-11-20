(: With copy-namespaces preserve, no-inherit, a constructor may place its
   content without a copy (the no-copy optimization at O1 and O2). Copying such
   a tree gives each element the in-scope namespaces a copy of the copied tree
   gives it. :)
declare copy-namespaces preserve, no-inherit;

declare function local:leaf()
{
  <b xmlns:q="urn:q"/>
};
declare function local:prefixes($t)
{
  for $e in $t/descendant-or-self::*
  return string-join(for $p in in-scope-prefixes($e) order by $p return $p, ",")
};

copy $c := <r xmlns:o="urn:o">{ local:leaf() }</r>
modify ()
return local:prefixes($c)
