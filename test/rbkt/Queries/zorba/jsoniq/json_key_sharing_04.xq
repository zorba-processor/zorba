(: A duplicate key in a large object. :)
import module namespace jn = "http://jsoniq.org/functions";

jn:parse-json("{ " || string-join(for $i in (1 to 100, 50) return '"k' || $i || '" : ' || $i, ", ") || " }")
