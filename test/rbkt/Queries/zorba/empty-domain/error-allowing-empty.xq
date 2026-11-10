(: The same with allowing empty: the domain is evaluated, not replaced by the empty sequence. :)
declare namespace e = "http://example.com/errors";

(: Recursive, so it is not inlined. :)
declare function local:fail($k as xs:integer) as empty-sequence()
{
  if ($k gt 0)
  then local:fail($k - 1)
  else fn:error(xs:QName("e:empty-domain"))
};

for $x allowing empty in local:fail(1)
return 1
