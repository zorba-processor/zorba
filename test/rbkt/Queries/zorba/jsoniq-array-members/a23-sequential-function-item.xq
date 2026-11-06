(: The main body is not sequential, but it calls sequential functions through
 : function items: the first rewrites the file after the first member is taken.
 : The members are the ones of the file as it was when parse-json began. :)
import module namespace file = "http://expath.org/ns/file";
import module namespace jn = "http://jsoniq.org/functions";
import module namespace random = "http://zorba.io/modules/random";
declare namespace an = "http://zorba.io/annotations";

declare variable $p := concat("jsoniq-array-members-", random:uuid(), ".json");

(: Larger than the buffer of the file stream. :)
declare function local:array($v as xs:string) as xs:string
{
  concat("[", string-join(for $i in 1 to 200000 return $v, ","), "]")
};

declare %an:sequential function local:init() as xs:string
{
  file:write-text($p, local:array("1"));
  $p
};

declare %an:sequential function local:rewrite() as empty-sequence()
{
  file:write-text($p, local:array("2"));
};

declare %an:sequential function local:delete() as empty-sequence()
{
  file:delete($p);
};

let $f := local:init#0()
let $sum := sum(
  for $m at $i in jn:members(jn:parse-json(file:read-text($f)))
  return ($m, if ($i eq 1) then local:rewrite#0() else ()))
return ($sum, local:delete#0())
