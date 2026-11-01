(: A .geojson file is a JSON document: dir:format and dir:open map it to json. :)
import module namespace dir = "http://zorba.io/modules/directory";
import module namespace jn = "http://jsoniq.org/functions";

declare variable $rbktPath as xs:string external;
declare variable $root := concat($rbktPath, "/Queries/zorba/directory/html-tree");

let $d := dir:open($root, "places.geojson")
return string-join((
  dir:format("a/b.GeoJSON"),
  $d("format"),
  string(exists($d("error"))),
  $d("content")("properties")("name"),
  string($d("content")("geometry")("coordinates")(2))
), " ")
