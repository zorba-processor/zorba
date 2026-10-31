(: A function item returned by zq:evaluate stays usable after
   zq:delete-query of the query that created it (QT3 inline-fn-032/-033
   crashed in the FOTS driver). :)
import module namespace zq = 'http://zorba.io/modules/zorba-query';

variable $key := zq:prepare-main-module(
  "function($a as xs:integer, $b as xs:integer) as xs:integer {$a + $b}");
variable $f := zq:evaluate($key);
zq:delete-query($key);
($f instance of function(xs:integer, xs:integer) as xs:integer,
 $f instance of function(xs:long, xs:long) as xs:integer+,
 $f instance of function(item()*, item()*) as xs:integer,
 $f(1, 2))
