(: One insert with attributes and other nodes: the attributes go to the
   target (into) or its parent (before, after), after its own attributes and
   in order; the other nodes go where the insert says. :)
declare function local:ins($kind as xs:string)
{
  copy $t := <a z="0">t<b/></a>
  modify
    switch ($kind)
    case "into" return
      insert nodes (attribute k { 1 }, attribute m { 2 }, <x/>, text { "u" })
        into $t
    case "first" return
      insert nodes (attribute k { 1 }, <x/>) as first into $t
    case "last" return
      insert nodes (attribute k { 1 }, <x/>) as last into $t
    case "before" return
      insert nodes (attribute k { 1 }, <x/>) before $t/b
    default return
      insert nodes (attribute k { 1 }, <x/>) after $t/b
  return $t
};

for $kind in ("into", "first", "last", "before", "after")
return (serialize(local:ins($kind)), "&#10;")
