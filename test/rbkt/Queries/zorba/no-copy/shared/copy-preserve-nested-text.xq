(: With copy-namespaces preserve, no-inherit, a constructor may place its
   content without a copy (the no-copy optimization at O1 and O2). Copying such
   a tree gives each element the in-scope namespaces a copy of the copied tree
   gives it. :)
declare copy-namespaces preserve, no-inherit;

declare function local:nest($n as xs:integer)
{
  if ($n eq 0) then text { "x" } else <a>{ local:nest($n - 1) }</a>
};

copy $c := local:nest(3) modify () return $c
