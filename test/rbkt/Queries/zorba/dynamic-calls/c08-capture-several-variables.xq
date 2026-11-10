(: A sequential inline function captures several variables and declares
   several local variables; each reads its own value. :)
declare namespace an = "http://zorba.io/annotations";
variable $a := "a";
variable $b := "b";
variable $c := "c";
variable $h := %an:sequential function() as xs:string
{
  variable $x := "x";
  variable $y := "y";
  variable $z := "z";
  string-join(($a, $b, $c, $x, $y, $z), "")
};
$h()
