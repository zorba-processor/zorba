(: A sequential inline function captures a variable declared in the main
   program and declares a local variable of its own. :)
declare namespace an = "http://zorba.io/annotations";
variable $prefix := "v";
variable $h := %an:sequential function() as xs:string
{
  variable $n := "1";
  concat($prefix, $n)
};
variable $r := $h();
($r, $h())
