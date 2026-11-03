jsoniq version "1.0";
(: Lets of navigations on $w, used once or more, across an order by. :)
let $a := [ {"v":1}, {"v":2}, {"v":3}, {"v":4} ]
return [
  for tumbling window $w in $a[]
    start at $i when true
    end at $j when $j - $i eq 1
  let $first := $w[1].v
  let $n := count($w)
  let $unused := $w[2].v
  order by $first descending
  return { "first": $first, "n": $n, "again": $first + $n }
]
