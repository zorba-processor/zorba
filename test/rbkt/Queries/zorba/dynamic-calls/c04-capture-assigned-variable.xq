(: An inline function captures a block variable that was assigned with :=
   before the function item was created. :)
declare namespace an = "http://zorba.io/annotations";
variable $prefix;
$prefix := "v";
variable $h := function($i) { concat($prefix, $i) };
variable $s := %an:sequential function($i) as xs:string
{
  variable $n := $i;
  concat($prefix, $n)
};
($h(1), $s(2))
