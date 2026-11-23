(: General = against a LET var where some pairs cannot be compared. The
   nested loop over the pairs raises the error for a probe that reaches such
   a pair before an equal item, and only then; it does so with the var hashed
   too. Each case is [var, probes]; the var is bound again for each case. :)
declare namespace an = "http://zorba.io/annotations";
declare namespace err = "http://www.w3.org/2005/xqt-errors";

declare %an:nondeterministic function local:cases()
{
  [ [ "a", "b" ], [ "a", "b", "c", 1 ] ],
  [ [ "a", 1 ], [ "a", "a", "a", "b" ] ],
  [ [ 1, "a" ], [ 1, 1, "a" ] ],
  [ [ true, false ], [ true, false, false, "true" ] ],
  [ [ true, false ], [ true, false, false, 1 ] ],
  [ [ "a", null ], [ null, "a", "x", null ] ],
  [ [ 1, null ], [ null, 1, 2, null, 1.5e0 ] ],
  [ [ true, null ], [ null, true, false ] ],
  [ [ "a", null ], [ null, "a", "x", 1 ] ],
  [ [ 1, 2.5, 3e0 ], [ 1, 2, 3, "3" ] ]
};

for $c in local:cases()
return {
  "probe = var" :
    string-join(
      let $set := $c(1)()
      for $p in $c(2)()
      return try { string($p = $set) } catch * { local-name-from-QName($err:code) },
      " "),
  "var = probe" :
    string-join(
      let $set := $c(1)()
      for $p in $c(2)()
      return try { string($set = $p) } catch * { local-name-from-QName($err:code) },
      " ")
}
