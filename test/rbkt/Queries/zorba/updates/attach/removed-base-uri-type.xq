(: Inserting an element takes off its hidden base-uri attribute, which is freed
   with the update list. The tree it was inserted into keeps no type for it:
   attributes inserted afterwards, which may take the memory of those freed,
   are untyped until revalidation gives them their type. :)
import schema namespace t = "http://zorba.io/tests/attach" at "typed-attribute.xsd";
variable $d := validate { <t:d/> };
for $i in 1 to 50 return insert node <t:e/> into $d;
for $i in 1 to 50 return insert node <t:e a="{$i}"/> into $d;
(count($d/t:e/@a), count($d/t:e/@a[. instance of attribute(*, xs:integer)]))
