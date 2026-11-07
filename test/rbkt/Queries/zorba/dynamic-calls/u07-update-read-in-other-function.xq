(: A function with no call through a function item counts nodes in a loop, and its result is consumed lazily by a loop that calls a sequential function through a function item, which inserts them: the count is not hoisted out of the function's loop, since the rule is for the whole query, not per body. :)
declare namespace an = "http://zorba.io/annotations";
declare variable $d := <a/>;
declare %an:sequential function local:add() { insert node <b/> into $d; };
declare variable $f := local:add#0;
declare function local:counts() { for $j in 1 to 3 return count($d/b) };
for $c in local:counts() return ($f(), $c)
