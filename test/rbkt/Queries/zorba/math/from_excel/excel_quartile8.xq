import module namespace
math="http://zorba.io/modules/math";

for $quart in 0 to 4
return math:quartile(5, $quart)
