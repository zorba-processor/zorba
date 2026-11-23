(: General = against the parameter of a function that is not inlined, called
   with a global and a local variable that are assigned new values between
   calls: each call sees the value the variable has then, also once the
   previous value was hashed. :)
declare variable $global := ("a", "b");

declare function local:in($p, $set, $depth)
{
  if ($depth gt 0) then local:in($p, $set, $depth - 1) else $p = $set
};

variable $local := (1, 2);

for $p in ("a", "b", "c", "a", "c", "b", "c", "d")
return {
  variable $r := concat($p, ":", string(local:in($p, $global, 1)),
                        ":", string(local:in(string-length($p), $local, 0)));
  if ($p eq "c") then
  {
    $global := if ($global = "c") then ("d", "e") else ("c", "x");
    $local := (2, 3);
  }
  else ();
  $r
}
