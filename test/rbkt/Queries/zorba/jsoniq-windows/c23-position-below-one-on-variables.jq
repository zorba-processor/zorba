jsoniq version "1.0";
(: XQuery 3.0 3.3.2: a position below 1 selects nothing, on any variable. :)
declare variable $g := (1, 2, 3);
let $x := (1, 2, 3)
return [ count($x), [ $x[0] ], [ $x[-1] ], [ $g[0] ], [ $g[-1] ] ]
