(: An inline function with an empty body returns the empty sequence, like a
   declared function with an empty body. It used to crash the translator
   (QT3 prod-FunctionCall/FunctionCall-044). :)
let $f := function($x) {}
return (count($f(2)), count((function() {})()))
