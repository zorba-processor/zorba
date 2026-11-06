import module namespace file = "http://expath.org/ns/file";
import module namespace csv = "http://zorba.io/modules/json-csv";

(: The file loses its mark between two reads of the same string: no text goes. :)
variable $f := concat(file:resolve-path("."), file:directory-separator(), "bom-rewrite.csv");
file:write-text($f, "&#xFEFF;a,b&#xA;1,2");
variable $s := file:read-text($f);
variable $first := string-join(jn:keys(csv:parse($s)), ",");
file:write-text($f, "c,d&#xA;3,4");
variable $second := string-join(jn:keys(csv:parse($s)), ",");
file:delete($f);
($first, $second)
