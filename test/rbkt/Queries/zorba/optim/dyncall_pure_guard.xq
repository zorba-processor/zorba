(: A query with no side effect: the call is not evaluated if nothing uses it (XQuery 3.0, 2.3.4). Was "none" before dynamic calls were kept in place. :)
declare function local:i($x as xs:integer) { $x };
declare variable $fs := (local:i#1, local:i#1);
declare variable $s := ();
let $r := $fs[1]($s) return if (exists($s)) then $r else "none"
