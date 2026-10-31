(:
 : Copyright 2006-2016 zorba.io
 :
 : Licensed under the Apache License, Version 2.0 (the "License");
 : you may not use this file except in compliance with the License.
 : You may obtain a copy of the License at
 :
 : http://www.apache.org/licenses/LICENSE-2.0
 :
 : Unless required by applicable law or agreed to in writing, software
 : distributed under the License is distributed on an "AS IS" BASIS,
 : WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 : See the License for the specific language governing permissions and
 : limitations under the License.
 :)

(:~
 : Converts the XML parse tree printed by "zorba --print-ast" into a Graphviz
 : DOT graph. Replaces the removed C++ parsenode_print_dot_visitor.
 :
 : Usage (-f: -q names a file; single module; the "AST for ..." header line is removed with grep):
 :
 :   zorba -f --parse-only --print-ast -q query.xq | grep -v '^AST for ' > ast.xml
 :   zorba -f -q scripts/ast2dot.xq -e ast=ast.xml -z method=text > ast.dot
 :   dot -Tsvg ast.dot -o ast.svg
 :
 : The external variable $ast is the parse tree as a document (or element)
 : whose root is ParseNodeTree. Each node is labelled with the element name
 : (the parse node class) followed by its attributes, one per line; the
 : source position (pos) and object address (ptr) are omitted so that the
 : graph is stable between runs. Labels are escaped for DOT.
 :)

declare namespace output = "http://www.w3.org/2010/xslt-xquery-serialization";
declare option output:method "text";

declare variable $ast external;

declare function local:esc($s as xs:string) as xs:string
{
  replace(
    replace(
      replace(replace($s, '\\', '\\\\'), '"', '\\"'),
      '&#10;', '\\n'),
    '&#13;', '')
};

declare function local:label($e as element()) as xs:string
{
  string-join(
    (local-name($e),
     for $a in $e/@*[not(local-name(.) = ('pos', 'ptr'))]
     return concat(local-name($a), '=', $a)),
    '&#10;')
};

declare function local:emit($e as element(), $id as xs:string) as xs:string*
{
  concat('  ', $id, ' [label="', local:esc(local:label($e)), '"];'),
  for $c at $i in $e/*
  let $cid := concat($id, '_', $i)
  return (concat('  ', $id, ' -> ', $cid, ';'), local:emit($c, $cid))
};

let $root := ($ast/descendant-or-self::ParseNodeTree)[1]
return
  string-join(
    ('digraph ast {',
     '  node [shape=box, fontname="Helvetica"];',
     local:emit($root, 'n'),
     '}',
     ''),
    '&#10;')
