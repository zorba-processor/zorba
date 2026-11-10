(: Inside a sequential function, inline functions capture a local variable,
   a parameter and a prolog variable; one is called in the function and one
   is returned and called by the caller. :)
declare namespace an = "http://zorba.io/annotations";
declare variable $sep := "-";
declare %an:sequential function local:make($p as xs:string)
{
  variable $prefix := concat($p, $sep);
  variable $f := function($i) { concat($prefix, $i) };
  variable $g := %an:sequential function($i) as xs:string
  {
    variable $n := $i;
    concat($prefix, $n)
  };
  ($f(1), $g)
};
variable $r := local:make("v");
($r[1], $r[2](2))
