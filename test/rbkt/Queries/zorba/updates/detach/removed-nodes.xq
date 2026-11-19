(: Nodes that updates remove become trees of their own once the updates are
   committed, with the namespaces and the base URI they had; text nodes made
   adjacent merge. Deleted, replaced (node and attribute) and replaced-content
   nodes, and one removed from a subtree that another update removes. :)
variable $x := <a xmlns:p="urn:p" xml:base="http://e/" k="1">t<b p:k="1"><p:c><d/></p:c></b>u<e>v<f/></e><g/></a>;
variable $b := $x/b;
variable $k := $x/@k;
variable $f := $x/e/f;
variable $d := $x/b/*/d;
variable $g := $x/g;
(
  delete node $b,
  delete node $d,
  replace node $k with attribute m { 2 },
  replace value of node $x/e with "w",
  replace node $g with <h/>
);
(
  serialize($x), count($x/text()),
  namespace-uri-for-prefix("p", $b), string(base-uri($b)), count($b//*),
  empty($d/..), string(base-uri($d)), namespace-uri-for-prefix("p", $d),
  empty($k/..), string($k),
  empty($f/..), string(base-uri($f)),
  empty($g/..), string(base-uri($g))
)
