(: General = against a global and a local variable that are assigned new
   values between comparisons: each comparison sees the value the variable
   has then, also once the previous value was hashed. :)
declare variable $global := ("a", "b");

variable $local := (1, 2);

for $p in ("a", "b", "c", "a", "c", "b", "c", "d")
return {
  variable $r := concat($p, ":", string($p = $global), "/", string($global = $p),
                        ":", string(string-length($p) = $local));
  if ($p eq "c") then
  {
    $global := if ($global = "c") then ("d", "e") else ("c", "x");
    $local := (2, 3);
  }
  else ();
  $r
}
