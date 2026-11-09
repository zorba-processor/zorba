(: A for clause whose domain has the empty sequence type evaluates the domain: its fn:error raises. :)
declare namespace e = "http://example.com/errors";

(: Recursive, so it is not inlined. :)
declare function local:fail($k as xs:integer) as empty-sequence()
{
  if ($k gt 0)
  then local:fail($k - 1)
  else fn:error(xs:QName("e:empty-domain"))
};

for $x in local:fail(1)
return 1
