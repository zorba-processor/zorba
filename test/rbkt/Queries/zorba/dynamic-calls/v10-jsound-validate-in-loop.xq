(: jsd:validate runs a constraint of the schema, a query from a string, that appends to the array it validates: the call runs once per tuple, and the size of the array beside it reads it per tuple. :)
import module namespace jsd = "http://jsound.io/modules/jsound";
import module namespace j = "http://jsoniq.org/functions";
declare variable $a := [ ];
declare variable $jsd := {
  "$namespace" : "http://zorba.io/tests/dynamic-calls-jsound",
  "$types" : [ {
    "$kind" : "array",
    "$name" : "t",
    "$content" : [ "integer" ],
    "$constraints" : [ "declare namespace an = ""http://zorba.io/annotations""; declare %an:sequential function local:add($a) { append json 1 into $a; true }; local:add($$)" ]
  } ]
};
for $i in 1 to 3 return (jsd:validate($jsd, "t", $a), j:size($a))
