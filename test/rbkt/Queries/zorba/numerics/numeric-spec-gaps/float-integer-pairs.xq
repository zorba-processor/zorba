(: Same rule as float-integer-reported.xq, in both input orders, for each integer
   and decimal value that rounds when cast to xs:float (F&O 3.0 19.1.2.1 casting
   through xs:string gives the nearest xs:float). Row: value:eq,
   distinct-values count both orders, group by count both orders. :)
declare function local:row($v as xs:anyAtomicType) as xs:string
{
  let $f := xs:float($v)
  return concat(string($v), ":", string($v eq $f),
    count(distinct-values(($v, $f))),
    count(distinct-values(($f, $v))),
    count(for $x in ($v, $f) group by $k := $x return $k),
    count(for $x in ($f, $v) group by $k := $x return $k))
};
for $v in (16777217, 33554435, 4294967297, 9007199254740994,
           -16777217, -33554435, -4294967297, -9007199254740994,
           xs:long("4294967297"), xs:unsignedInt("4294967295"),
           xs:unsignedLong("9007199254740994"),
           16777217.0, 4294967297.0, 16777216.5, 4294967297.5)
return local:row($v)
