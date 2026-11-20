(: Inserting a node one level past the node id limit raises an error and leaves
   the copied tree intact (it crashed while the query closed). :)
declare namespace err = "http://www.w3.org/2005/xqt-errors";
declare namespace zerr = "http://zorba.io/errors";

declare function local:doc($depth as xs:integer) as xs:string
{
  string-join((for $i in 1 to $depth return "<a>",
               for $i in 1 to $depth return "</a>"))
};

(
  copy $c := parse-xml(local:doc(1017))
  modify insert node <x/> into ($c//a)[last()]
  return count($c//x)
,
  try
  {
    copy $c := parse-xml(local:doc(1018))
    modify insert node <x/> into ($c//a)[last()]
    return count($c//x)
  }
  catch zerr:ZSTR0030 | err:XPDY0130
  {
    "node id limit"
  }
)
