(: namespace-node() kind tests on axes that never reach namespace nodes: the
   static typing of these steps used to fail with an internal assertion. :)
let $a := <a xmlns:p="urn:p"><b/><c/></a>
return (
  count($a/namespace-node()),
  count($a//namespace-node()),
  count($a/descendant-or-self::namespace-node()),
  count($a/b/following-sibling::namespace-node()),
  count($a/c/preceding::namespace-node()),
  count($a/b/ancestor::namespace-node()),
  count($a/b/ancestor-or-self::namespace-node()),
  count($a/namespace-node()[b]/c)
)
