xquery version "3.0";
declare namespace err = "http://www.w3.org/2005/xqt-errors";

(: Errors of inserts that mix attributes and other nodes: an attribute after
   another node (XUTY0004), an attribute the target has (XUDY0021), a prefix
   bound to another namespace on the target (XUDY0023), and attributes before
   a child of a document (XUDY0030). :)
declare function local:code($f as function() as item()*) as xs:string
{
  try { string-join(for $i in $f() return serialize($i), " ") }
  catch * { local-name-from-QName($err:code) }
};

local:code(function() {
  copy $t := <a/> modify insert nodes (<x/>, attribute k { 1 }) into $t return $t
}),
local:code(function() {
  copy $t := <a k="0"/> modify insert nodes (attribute k { 1 }, <x/>) into $t
  return $t
}),
local:code(function() {
  copy $t := <a xmlns:p="urn:one"/>
  modify insert nodes (attribute { QName("urn:two", "p:k") } { 1 }, <x/>)
           into $t
  return $t
}),
local:code(function() {
  copy $t := document { <b/> }
  modify insert nodes (attribute k { 1 }, <x/>) before $t/b
  return $t
})
