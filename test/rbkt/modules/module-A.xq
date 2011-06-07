module namespace modA = "http://www.zorba-xquery.com/modules/A";

(: This module exists only to import a specific version of the "ver" module. :)

import module namespace vm = "http://www.zorba-xquery.com/modules/ver#1.0";

declare namespace ver = "http://www.zorba-xquery.com/versioning";
declare option ver:version "1.0";

declare function modA:hello() as xs:string {
  vm:hello()
};