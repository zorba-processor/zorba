jsoniq version "1.0";
(: Baseline: the cost of the input alone, no window. :)
import module namespace b = "http://zorba.io/bench/windows" at "lib.xqm";
declare variable $source as string external;
sum(for $x in b:source($source) return b:numbers($x))
