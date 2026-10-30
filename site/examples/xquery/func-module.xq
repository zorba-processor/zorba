import module namespace geo = "http://example.org/geo" at "lib/geo.xqm";

(: Mexico City to Guadalajara, rounded to whole kilometres :)
round(geo:distance(19.4326, -99.1332, 20.6597, -103.3496))
