(: unordered-maps.xq map:create: "@error err:XPTY0004 if any of the key attribute
   types is not a subtype of anyAtomicType", and map:insert: "@error zerr:ZXQD0005 if
   any of the given key attributes can not be cast (or is not a subtype) of the
   corresponding key attribute specified when creating the map." xs:integer is a
   subtype of xs:anyAtomicType. :)
import module namespace map = "http://zorba.io/modules/unordered-maps";
variable $r := ();
map:create("msg-any-q", xs:QName("xs:anyAtomicType"));
map:insert("msg-any-q", 1, "one");
map:insert("msg-any-q", "a", "letter");
map:insert("msg-any-q", 2.5e0, "double");
$r := (map:get("msg-any-q", 1), map:get("msg-any-q", "a"), map:get("msg-any-q", 2.5e0), map:get("msg-any-q", 1e0), map:size("msg-any-q"));
map:drop("msg-any-q");
$r
