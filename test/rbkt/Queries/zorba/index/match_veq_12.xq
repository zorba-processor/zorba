(: The index is matched against the where clause before the count clause.
   The where clause after the count clause stays in the query; it does not
   decide whether the index applies. :)
import module namespace items = "www.items.com" at "match_veq_12.xqlib";

import module namespace ddl = 
  "http://zorba.io/modules/store/static/collections/ddl";

import module namespace dml = 
  "http://zorba.io/modules/store/static/collections/dml";

import module namespace iddl = 
  "http://zorba.io/modules/store/static/indexes/ddl";


declare variable $doc := 
<items>
<item id="a"/>
<item id="b"/>
<item id="c"/>
</items>;


ddl:create(xs:QName("items:items"));

iddl:create(xs:QName("items:items-index"));

for $item in $doc/item
return dml:insert(xs:QName("items:items"), $item);

for $x in dml:collection(xs:QName("items:items"))
where xs:string($x/@id) eq "b"
count $c
where $c ge 1
return <r c="{$c}">{$x}</r>
