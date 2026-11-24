(: A coercion has the name of the function it coerces, also when that function
   is known only at run time :)
declare function local:coerce($arg as function(item()*) as item()*)
  as function(item()*) as item()*
{
  $arg
};

declare function local:coerce-twice($arg as function(item()*) as item()*)
  as function(item()*) as item()*
{
  local:coerce($arg)
};

declare function local:id($x as item()*) as item()*
{
  $x
};

(
  fn:function-name(local:coerce(fn:abs#1))
    eq fn:QName("http://www.w3.org/2005/xpath-functions", "fn:abs"),
  fn:function-name(local:coerce(local:id#1))
    eq fn:QName("http://www.w3.org/2005/xquery-local-functions", "local:id"),
  fn:function-name(local:coerce-twice(fn:abs#1))
    eq fn:QName("http://www.w3.org/2005/xpath-functions", "fn:abs"),
  fn:empty(fn:function-name(local:coerce(function($x) { $x }))),
  fn:empty(fn:function-name(local:coerce(fn:concat("a", ?))))
)
