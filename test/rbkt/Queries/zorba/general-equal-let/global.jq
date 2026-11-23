(: General = against a global variable, compared again with the same value:
   hashed from its second comparison on, with the result and the errors of
   the nested loop over the pairs (an error only for a probe that reaches a
   pair that cannot be compared before an equal item). :)
declare namespace an = "http://zorba.io/annotations";
declare namespace err = "http://www.w3.org/2005/xqt-errors";

declare %an:nondeterministic function local:strings() { "a", "b", "c", "a" };
declare %an:nondeterministic function local:numbers() { 1, 2.5, 3e0, 4, 0.1, 9007199254740993, -0e0 };

declare variable $strings := local:strings();
declare variable $numbers := local:numbers();
declare variable $mixed := ("a", 1);
declare variable $nulls := ("a", null);
declare variable $none := local:strings()[$$ eq "z"];

declare function local:compare($probes, $set)
{
  string-join(
    for $p in $probes
    return try { string($p = $set) || "/" || string($set = $p) }
           catch * { local-name-from-QName($err:code) },
    " ")
};

{ "strings" :
    string-join(for $p in ("a", "x", "c", "b", "a", "y")
                return string($p = $strings) || "/" || string($strings = $p), " "),
  "numbers" :
    string-join(for $p in (1e0, 2.5e0, 3, 4.0, 5, 1e-1, 9007199254740992e0, 9007199254740992, 0, double("NaN"))
                return string($p = $numbers) || "/" || string($numbers = $p), " "),
  "mixed" :
    string-join(for $p in ("a", "a", 1, "b", 2)
                return try { string($p = $mixed) } catch * { local-name-from-QName($err:code) }, " "),
  "mixed, var first" :
    string-join(for $p in ("a", "a", 1, "b", 2)
                return try { string($mixed = $p) } catch * { local-name-from-QName($err:code) }, " "),
  "nulls" :
    string-join(for $p in (null, "a", "x", null, 1)
                return try { string($p = $nulls) } catch * { local-name-from-QName($err:code) }, " "),
  "empty" :
    string-join(for $p in ("a", "b", 1) return string($p = $none) || "/" || string($none = $p), " "),
  "strings against numbers" :
    string-join(for $p in ("a", "b", "c")
                return try { string($p = $numbers) } catch * { local-name-from-QName($err:code) }, " "),
  "in a function" : local:compare(("a", "x", "c", 1), $strings)
}
