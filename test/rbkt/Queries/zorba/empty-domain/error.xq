(: A for clause whose domain has the empty sequence type evaluates the domain: its fn:error raises. :)
declare namespace e = "http://example.com/errors";

declare function local:fail() as empty-sequence()
{
  fn:error(xs:QName("e:empty-domain"))
};

for $x in local:fail()
return 1
