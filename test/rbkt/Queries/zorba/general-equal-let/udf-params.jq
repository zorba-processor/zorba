(: General = against the parameter of a function that is not inlined (it
   recurses), called again with the same value of the caller's variable:
   hashed from its second call with that value on, with the result and the
   errors of the nested loop over the pairs. A new binding of the caller's
   variable (an outer for, a function called with another argument) is
   compared anew. :)
declare namespace an = "http://zorba.io/annotations";
declare namespace err = "http://www.w3.org/2005/xqt-errors";

declare %an:nondeterministic function local:strings() { "a", "b", "c", "a" };
declare %an:nondeterministic function local:numbers() { 1, 2.5, 3e0, 4, 0.1, 9007199254740993, -0e0 };

declare variable $global := local:strings();

(: $depth recursive calls pass $set on before it is compared. :)
(: $p[1], not a var reference, leaves $set the hashed operand of $set = $p[1]. :)
declare function local:in($p, $set, $depth)
{
  if ($depth gt 0)
  then local:in($p, $set, $depth - 1)
  else try { string($p = $set) || "/" || string($set = $p[1]) }
       catch * { local-name-from-QName($err:code) }
};

declare function local:typed($p as xs:string, $set as xs:string*)
{
  if (string-length($p) gt 5) then local:typed(substring($p, 2), $set) else $p = $set
};

declare function local:each($probes, $set, $depth)
{
  string-join(for $p in $probes return local:in($p, $set, $depth), " ")
};

{ "strings" :
    let $set := local:strings()
    return local:each(("a", "x", "c", "b", "a", "y"), $set, 0),
  "strings, passed on" :
    let $set := local:strings()
    return local:each(("a", "x", "c", "b", "a", "y"), $set, 2),
  "numbers" :
    let $set := local:numbers()
    return local:each((1e0, 2.5e0, 3, 4.0, 5, 1e-1, 9007199254740992e0, 9007199254740992, 0,
                       double("NaN"), xs:float(2.5)), $set, 1),
  "mixed" :
    let $set := (local:strings()[1], 1)
    return local:each(("a", "a", 1, "b", 2, "b"), $set, 0),
  "nulls" :
    let $set := (local:strings()[1], null)
    return local:each((null, "a", "x", null, 1, 1), $set, 0),
  "strings against numbers" :
    let $set := local:numbers()
    return local:each(("a", "b", 1, "c"), $set, 1),
  "untyped" :
    let $set := (local:strings(), xs:untypedAtomic("1"))
    return local:each((xs:untypedAtomic("a"), "b", 1, xs:untypedAtomic("z"), 2), $set, 0),
  "empty" :
    let $set := local:strings()[$$ eq "z"]
    return local:each(("a", "b", 1), $set, 0),
  "outer for" :
    string-join(
      for $s in ("a", "b", "c")
      let $set := local:strings()[$$ ne $s]
      return local:each(("a", "b", "c", "a"), $set, 0),
      " | "),
  "global" : local:each(("a", "x", "c", "a"), $global, 1),
  "positional" :
    let $set := local:strings()
    return local:each(("a", "b", "c", "b"), $set[2], 0),
  "sequence" :
    let $set := local:strings()
    return local:each(("a", "z", "c", "z"), ($set, "z"), 0),
  "typed" :
    let $set := local:strings()
    return string-join(for $p in ("a", "x", "bbbbbbb", "a") return string(local:typed($p, $set)), " "),
  "alternating" :
    let $s1 := local:strings()
    let $s2 := local:numbers()
    return string-join(for $p in ("a", 1, "b", 4, "x", 5)
                       return local:in($p, $s1, 0) || "," || local:in($p, $s2, 0), " ")
}
