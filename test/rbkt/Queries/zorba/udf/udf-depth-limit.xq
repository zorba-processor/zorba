(: Recursion deeper than the default call depth limit of 1024 raises
   ZXQP0069 instead of overflowing the stack. :)
declare function local:depth($n as xs:integer) as xs:integer
{
  if ($n le 0) then 0 else 1 + local:depth($n - 1)
};

local:depth(5000)
