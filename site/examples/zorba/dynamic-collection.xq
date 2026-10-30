import module namespace ddl = "http://zorba.io/modules/store/dynamic/collections/ddl";
import module namespace dml = "http://zorba.io/modules/store/dynamic/collections/dml";

variable $notes := xs:QName("notes");

ddl:create($notes);
dml:insert($notes, (<note n="1">buy milk</note>, <note n="2">call mom</note>));
dml:insert-last($notes, <note n="3">fix bug</note>);
delete node dml:collection($notes)[@n = "2"];

<notes count="{count(dml:collection($notes))}">{ dml:collection($notes) }</notes>
