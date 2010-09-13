import module namespace yahoo = 'http://www.zorba-xquery.com/modules/webservices/yahoo/maps';
import module namespace shared="http://www.zorba-xquery.com/modules/webservices/yahoo/shared";

let $location := "701 First Avenue, Sunnyvale, CA"
let $response := yahoo:map($location)
return
  if(empty($response))
  then <test name="map" succeed="false" message="Location not found." />
  else <test name="map" succeed="true" message="Location found." />