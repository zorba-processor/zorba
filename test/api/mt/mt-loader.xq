xquery version "3.0";

(: An external module that mt_dynamic_loader loads from a shared library. :)
module namespace m = "http://zorba-tests.example.org/modules/mt-loader";

declare namespace ver = "http://zorba.io/options/versioning";
declare option ver:module-version "1.0";

declare function m:digit($name as xs:string) as xs:integer? external;
