(: Sequences never nest: inner parentheses just concatenate. :)
let $s := (1, (2, 3), (), ("four", (5)))
return (count($s), $s)
