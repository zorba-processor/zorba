(: unordered-maps.xq map:create: "@error err:XPTY0004 if any of the key attribute
   types is not a subtype of anyAtomicType", and map:insert: "@error zerr:ZXQD0005 if
   any of the given key attributes can not be cast (or is not a subtype) of the
   corresponding key attribute specified when creating the map." xs:integer is a
   subtype of xs:anyAtomicType. :)
import module namespace map = "http://zorba.io/modules/unordered-maps";
variable $r := ();
map:create("msg-any-s", "anyAtomicType");
map:insert("msg-any-s", 1, "one");
map:insert("msg-any-s", "a", "letter");
map:insert("msg-any-s", 2.5e0, "double");
$r := (map:get("msg-any-s", 1), map:get("msg-any-s", "a"), map:get("msg-any-s", 2.5e0), map:get("msg-any-s", 1e0), map:size("msg-any-s"));
map:drop("msg-any-s");
$r
