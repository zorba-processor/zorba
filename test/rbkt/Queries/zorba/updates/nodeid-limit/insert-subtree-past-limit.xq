(: An inserted subtree whose root fits but whose child exceeds the node id
   limit raises an error before the target tree changes. :)
declare namespace err = "http://www.w3.org/2005/xqt-errors";
declare namespace zerr = "http://zorba.io/errors";

declare function local:doc($depth as xs:integer) as xs:string
{
  string-join((for $i in 1 to $depth return "<a>",
               for $i in 1 to $depth return "</a>"))
};

(
  copy $c := parse-xml(local:doc(1016))
  modify insert node <x><y/></x> into ($c//a)[last()]
  return count($c//y)
,
  try
  {
    copy $c := parse-xml(local:doc(1017))
    modify insert node <x><y/></x> into ($c//a)[last()]
    return count($c//y)
  }
  catch zerr:ZSTR0030 | err:XPDY0130
  {
    "node id limit"
  }
)
