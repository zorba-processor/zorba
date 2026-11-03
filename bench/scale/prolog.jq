jsoniq version "1.0";
import module namespace dir = "http://zorba.io/modules/directory";
import module namespace file = "http://expath.org/ns/file";
import module namespace csv = "http://zorba.io/modules/json-csv";
declare function local:num($x as item*) as item* { $x[$$ instance of decimal] };
declare function local:t($x as item) as xs:dateTime? { xs:dateTime($x.ts) };
