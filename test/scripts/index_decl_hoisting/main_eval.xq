import module namespace t = "http://zorba.io/test/index-decl-hoisting" at "module.xqlib";
import module namespace r = "http://zorba.io/modules/reflection";

(: An eval is a call that the compiler cannot prove free of side effects: this query moves nothing out of a loop. :)
r:eval("1"),
let $doc := <root>
              <foo id="-1"><boo id="1"/><boo id="2"/></foo>
              <foo id="-2"><boo id="3"/><boo id="4"/></foo>
            </root>
for $a in $doc/foo
for $b in $a//boo
return
{
  variable $x := $b/@id;

  for $c in 1 to $x
  return $a/@id + 2 * $x + $c
}
