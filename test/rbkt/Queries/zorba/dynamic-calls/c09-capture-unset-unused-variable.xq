(: An inline function is created while a block variable without a value is
   in scope; its body does not use the variable. :)
variable $unset;
variable $h := function($i) { $i + 1 };
$h(1)
