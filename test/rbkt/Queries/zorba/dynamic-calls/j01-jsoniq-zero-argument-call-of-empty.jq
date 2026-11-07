jsoniq version "1.0";
(: In JSONiq, a call with no argument over an empty sequence is empty at every level, also when inlining shows that the sequence has at most one item. :)
declare function local:m($b) { if ($b) then { "a": 1 } else () };
local:m(false)()
