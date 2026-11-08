(: The message of XUDY0014 names what the target is: a node, a collection, an
   index, an integrity constraint or a document. :)
import module namespace t = "http://zorba.io/tests/transform-targets" at "targets.xqlib";
import module namespace ddl = "http://zorba.io/modules/store/static/collections/ddl";
import module namespace iddl = "http://zorba.io/modules/store/static/indexes/ddl";
import module namespace icddl = "http://zorba.io/modules/store/static/integrity-constraints/ddl";
import module namespace doc = "http://zorba.io/modules/store/documents";
declare namespace err = "http://www.w3.org/2005/xqt-errors";
ddl:create(xs:QName("t:c"));
doc:put("http://zorba.io/tests/transform-targets/doc.xml", document { <a/> });
let $outside := <o/>
return string-join((
  try { copy $c := <x/> modify rename node $outside as "z" return $c }
  catch * { $err:code || ": " || $err:description },
  try { copy $c := <x/> modify ddl:delete(xs:QName("t:c")) return $c }
  catch * { $err:code || ": " || $err:description },
  try { copy $c := <x/> modify iddl:create(xs:QName("t:idx")) return $c }
  catch * { $err:code || ": " || $err:description },
  try { copy $c := <x/> modify icddl:activate(xs:QName("t:ic")) return $c }
  catch * { $err:code || ": " || $err:description },
  try { copy $c := <x/> modify doc:remove("http://zorba.io/tests/transform-targets/doc.xml") return $c }
  catch * { $err:code || ": " || $err:description }), "&#10;")
