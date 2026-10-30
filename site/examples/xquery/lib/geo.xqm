module namespace geo = "http://example.org/geo";

declare namespace math = "http://www.w3.org/2005/xpath-functions/math";

declare variable $geo:earth-radius-km := 6371;

declare function geo:deg-to-rad($d as xs:double) as xs:double
{
  $d * math:pi() div 180
};

(: Great-circle distance in km between two (lat, lon) points. :)
declare function geo:distance($lat1 as xs:double, $lon1 as xs:double,
                              $lat2 as xs:double, $lon2 as xs:double)
  as xs:double
{
  let $dlat := geo:deg-to-rad($lat2 - $lat1)
  let $dlon := geo:deg-to-rad($lon2 - $lon1)
  let $a := math:pow(math:sin($dlat div 2), 2)
            + math:cos(geo:deg-to-rad($lat1)) * math:cos(geo:deg-to-rad($lat2))
              * math:pow(math:sin($dlon div 2), 2)
  return 2 * $geo:earth-radius-km * math:asin(math:sqrt($a))
};
