(: Equality join with a table of 64 rows, written in the query. :)
let $table := for $i in 0 to 63 return { "node" : "n" || $i, "rack" : $i mod 8 }
return sum(for $r in @INPUT@
           for $t in $table
           where $t.node eq $r.node
           return $t.rack)
