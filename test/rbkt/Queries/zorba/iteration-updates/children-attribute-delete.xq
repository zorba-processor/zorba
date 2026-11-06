(: Behavior check (the update runs in a let clause, which already gave this result before the store change): A sequential function called in a let clause deletes an attribute of the node whose children the for clause iterates. :)
declare namespace an = "http://zorba.io/annotations";
declare variable $d := <a x="1" y="2" z="3"><b k="1"/><b k="2"/><b k="3"/></a>;
declare %an:sequential function local:del() { delete node $d/@*[1]; };
for $y in $d/b
let $u := local:del()
return string($y/@k)
