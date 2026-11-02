(: Objects of one parse share key items: updates on one object must not
   change the keys or pairs of the others. :)
import module namespace jn = "http://jsoniq.org/functions";

variable $a := jn:parse-json(
  '[ { "id" : 1, "name" : "a" }, { "id" : 2, "name" : "b" },
     { "id" : 3, "name" : "c" }, { "name" : "d", "id" : 4 } ]');

rename json $a[[1]].id as "key";
delete json $a[[2]].name;
insert json { "extra" : true } into $a[[3]];
replace value of json $a[[4]].id with 40;

string-join(
  for $o in $a[]
  return string-join(for $k in jn:keys($o) return $k || "=" || $o.$k, ","),
  ";")
