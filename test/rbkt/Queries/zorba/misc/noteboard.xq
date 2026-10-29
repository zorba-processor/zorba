import module namespace ddl = "http://zorba.io/modules/store/static/collections/ddl";

import module namespace nb = "http://zorba-tests.example.org/noteboard" at "noteboard.xqlib";

declare variable $local:action as xs:string := "recent";

ddl:create($nb:notes);

variable $local:out :=
  switch ($local:action)
  case "recent" return nb:recent(3)
  case "post" return nb:post("hello")
  default return fn:error(
    fn:QName("http://zorba-tests.example.org/noteboard", "nb:unknown-action"),
    concat("unknown action: ", $local:action));

fn:serialize($local:out)
