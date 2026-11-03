(: The average of v per node (64 groups), then the sum of those averages. :)
sum(for $r in @INPUT@
    let $k := $r.node
    let $v := local:num($r.v)
    group by $k
    return avg($v))
