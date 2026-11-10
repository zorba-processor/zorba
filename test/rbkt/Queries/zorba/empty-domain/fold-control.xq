(: Control: a later let clause keeps every tuple, so the one-item for variable is folded into the return clause. :)
for $a in (1, 2) ! (6 idiv .)
let $b := 10
return $a + $b
