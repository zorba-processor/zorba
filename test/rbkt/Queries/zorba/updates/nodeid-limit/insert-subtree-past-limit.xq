(: Inserting a subtree whose child lands past the former node id limit
   (about 1 018 levels) works: node ids have no fixed length. :)
declare function local:doc($depth as xs:integer) as xs:string
{
  string-join((for $i in 1 to $depth return "<a>",
               for $i in 1 to $depth return "</a>"))
};

for $depth in (1016, 1017)
return
  copy $c := parse-xml(local:doc($depth))
  modify insert node <x><y/></x> into ($c//a)[last()]
  return count($c//y)
