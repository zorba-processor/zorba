(: xs:float values above 2^56 are all multiples of 2^33; they are distinct values
   (F&O 3.0 op:numeric-equal) and must stay so. :)
let $floats := for $i in 1 to 2000 return xs:float($i) * xs:float(72057594037927936)
return (count(distinct-values($floats)), count(for $x in $floats group by $k := $x return $k))
