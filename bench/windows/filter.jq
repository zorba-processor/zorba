jsoniq version "1.0";
(: A filter: the records with a number in "v". :)
import module namespace b = "http://zorba.io/bench/windows" at "lib.xqm";
declare variable $source as string external;
count(for $x in b:source($source) where exists(b:numbers($x)) return 1)
