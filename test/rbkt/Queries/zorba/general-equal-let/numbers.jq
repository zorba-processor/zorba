(: General = between numbers and a LET var of numbers, as parse-json gives
   them: xs:integer, xs:decimal and xs:double compare by value, the var hashed
   from its second comparison on. 0.1 as xs:decimal equals 0.1 as xs:double
   (cast to xs:double) but not 0.1000000000000000000001, which has the same
   xs:double value; 9007199254740993 equals 9007199254740992e0 (cast to
   xs:double) but not 9007199254740992; NaN equals nothing; -0 equals 0. :)
declare namespace an = "http://zorba.io/annotations";

declare %an:nondeterministic function local:set()
{
  1, 2.5, 3e0, 4, 0.1, 9007199254740993, -0e0, 2.5, 1
};

declare %an:nondeterministic function local:small()
{
  1, 2.5, 3e0, 0.1, -0e0, 2.5, double("NaN")
};

declare %an:nondeterministic function local:probes()
{
  1e0, 2.5e0, 3, 4.0, 5, 1e-1, 0.1000000000000000000001,
  9007199254740992e0, 9007199254740992, 0, 0.0, 1.0, double("NaN"), float(1)
};

let $set := local:set()
for $p in local:probes()
return string($p = $set) || "/" || string($set = $p),

let $set := local:small()
for $p in local:probes()
return string($p = $set) || "/" || string($set = $p)
