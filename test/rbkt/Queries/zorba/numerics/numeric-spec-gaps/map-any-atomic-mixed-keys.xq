(: unordered-maps.xq map:get: "getting the entries is based on the "eq" comparison".
   F&O 3.0 op:numeric-equal: 5 eq 5.5e0 is false, 5 eq 5e0 is true. Many keys, so that
   integers, fractional doubles and strings meet in the same hash bucket; values of
   types eq does not compare are not equal. :)
import module namespace map = "http://zorba.io/modules/unordered-maps";
variable $r := ();
map:create("msg-mix", xs:QName("xs:anyAtomicType"));
for $i in 1 to 3000
return (
  map:insert("msg-mix", $i, "i"),
  map:insert("msg-mix", $i + 0.5e0, "d"),
  map:insert("msg-mix", string($i), "s")
);
map:insert("msg-mix", 16777217, "big");
$r := (
  map:size("msg-mix"),
  map:get("msg-mix", 5), map:get("msg-mix", 5.5e0), map:get("msg-mix", "5"),
  map:get("msg-mix", xs:float(5.5)), map:get("msg-mix", 5.5),
  map:get("msg-mix", xs:float(16777216))
);
map:drop("msg-mix");
$r
