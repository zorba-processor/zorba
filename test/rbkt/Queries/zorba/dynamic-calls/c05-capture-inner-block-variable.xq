(: An inline function captures a variable of an inner block and is called
   inside the block and after the block has completed. :)
declare namespace an = "http://zorba.io/annotations";
variable $f := ();
variable $inside := ();
{
  variable $prefix := "v";
  $f := %an:sequential function($i) as xs:string
  {
    variable $n := $i;
    concat($prefix, $n)
  };
  $inside := $f(1);
}
($inside, $f(2))
