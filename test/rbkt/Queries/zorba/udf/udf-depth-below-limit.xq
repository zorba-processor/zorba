(: 1001 nested calls, below the default call depth limit of 1024: must not
   raise ZXQP0069 nor exhaust the machine stack. :)
declare function local:depth($n as xs:integer) as xs:integer
{
  if ($n le 0) then 0 else 1 + local:depth($n - 1)
};

local:depth(1000)
