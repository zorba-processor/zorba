(: A write to the file does not change the members in progress. :)
import module namespace file = "http://expath.org/ns/file";
import module namespace jn = "http://jsoniq.org/functions";
import module namespace random = "http://zorba.io/modules/random";

variable $p := concat("jsoniq-array-members-", random:uuid(), ".json");
variable $out := 0;

file:write-text($p, "[1,2,3]");

for $m in jn:members(jn:parse-json(file:read-text($p), { "zorba-streaming-members" : true() }))
return
{
  file:write-text($p, "[7,8,9]");
  $out := $out * 10 + $m;
}

file:delete($p);

$out
