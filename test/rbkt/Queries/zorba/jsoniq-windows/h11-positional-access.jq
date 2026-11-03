jsoniq version "1.0";
(: Positional functions and predicates on $w, which rewrites can simplify. :)
let $a := [1, 2, 3]
return [
  for tumbling window $w in $a[]
    start when true
    end when false
  return [ head($w), [ tail($w) ], $w[last()], [ $w[position() gt 1] ], $w[2], subsequence($w, 2, 1), [ reverse($w) ] ]
]
