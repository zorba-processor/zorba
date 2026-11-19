(: An inline function uses a variable only in an inline function nested in
   its body; a block variable without a value is also in scope. :)
variable $unset;
variable $prefix := "v";
variable $h := function() { function($i) { concat($prefix, $i) } };
$h()(1)
