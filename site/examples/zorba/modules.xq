import module namespace string = "http://zorba.io/modules/string";
import module namespace math = "http://zorba.io/modules/math";
import module namespace seq = "http://zorba.io/modules/sequence";
import module namespace ft = "http://zorba.io/modules/full-text";

(
  <split>{ string:split("a,b,,c", ",") ! concat("[", ., "]") }</split>,
  <roman>{ math:roman(2026) }</roman>,
  <union>{ seq:value-union((1, 2, 3), (2, 3, 4)) }</union>,
  <stem>{ ft:stem("running", xs:language("en")) }</stem>
)
