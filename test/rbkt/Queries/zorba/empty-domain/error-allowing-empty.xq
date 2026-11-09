(: The same with allowing empty: the domain is evaluated, not replaced by the empty sequence. :)
declare namespace e = "http://example.com/errors";

declare function local:fail() as empty-sequence()
{
  fn:error(xs:QName("e:empty-domain"))
};

for $x allowing empty in local:fail()
return 1
