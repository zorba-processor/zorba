(: General = against a LET var with an empty side: an empty probe (a missing
   key) or an empty var is false, and the other side is not compared, so a
   var of items that the probe could not be compared with raises nothing. :)
declare namespace an = "http://zorba.io/annotations";

declare %an:nondeterministic function local:events()
{
  { "id" : "a" }, {}, { "id" : "b" }, {}, { "id" : 1 }
};

let $ids := local:events()[$$.id instance of string].id
for $e in local:events()[not($$.id instance of integer)]
return $e.id = $ids,

let $none := local:events()[$$.id instance of boolean].id
for $e in local:events()
return ($e.id = $none, $none = $e.id),

let $numbers := local:events()[$$.id instance of integer].id
for $e in local:events()[empty($$.id)]
return ($e.id = $numbers, $numbers = $e.id)
