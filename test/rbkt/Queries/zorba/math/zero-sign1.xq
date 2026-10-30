(: The sign of a zero argument: acos(-0) is +pi/2; asin, atan, sqrt,
   asinh and atanh keep the sign. :)
import module namespace zm = "http://zorba.io/modules/math";
declare namespace math = "http://www.w3.org/2005/xpath-functions/math";

(math:acos(-0.0e0), math:asin(-0.0e0), math:atan(-0.0e0), math:sqrt(-0.0e0),
 zm:asinh(-0.0e0), zm:atanh(-0.0e0))
