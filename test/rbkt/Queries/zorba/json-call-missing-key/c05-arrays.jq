jsoniq version "1.0";
(: A call on a missing key, a missing member, and a member then a key. :)
for $e in ({ "a" : [ { "k" : 1 }, { "k" : 2 } ] }, { "t" : 1 }, { "a" : [ { "j" : 3 } ] })
where $e.a(1)("k") ge 1 or empty($e.a(2))
return [ $e.a(2), $e.a(1)("k") ]
