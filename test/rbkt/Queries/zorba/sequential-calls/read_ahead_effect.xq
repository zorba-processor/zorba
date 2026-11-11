(: A sequential function declared to return at most one item runs to its end
   even when the caller needs only its first item: the step after the item
   runs. :)
declare namespace an = "http://zorba.io/annotations";

declare %an:assignable variable $steps := 0;

declare %an:sequential function local:item-then-step() as xs:integer?
{
  for $i in (1, 2)
  return
    if ($i eq 1)
    then $i
    else { $steps := $steps + 1; () }
};

variable $exists := fn:exists(local:item-then-step());
fn:concat($exists, " ", $steps)
