(: A chain of $levels nested elements, each with a preceding and a following
   sibling: loaded, walked, compared, copied and serialized. :)
declare variable $levels external := "1000";
declare variable $n := xs:integer($levels);

let $s := string-join((for $i in 1 to $n return "<a><p/>",
                       "x",
                       for $i in 1 to $n return "<q/></a>"))
let $doc := parse-xml($s)
let $deepest := ($doc//p)[last()]
return (
  count($doc//a),
  count($deepest/ancestor::*),
  count($doc//q[. >> $deepest]),
  count(document { $doc }//p),
  string-length(serialize($doc))
)
