(: XQUF updates on parsed elements whose attribute and child count crosses
   zero, one, two and three, including a rolled-back update. Node identity
   and document order must survive every step. :)

declare function local:check($name as xs:string, $ok as xs:boolean)
  as xs:string
{
  concat($name, ": ", $ok)
};

variable $doc := parse-xml('<r><s>x</s><t k="v">y</t><e/></r>');
variable $r := $doc/r;
variable $s := $r/s;
variable $t := $r/t;
variable $e := $r/e;
variable $x := $s/text();
variable $out := ();

insert node attribute a { "1" } into $s;
$out := ($out, local:check("insert attribute next to one child",
  deep-equal($s, <s a="1">x</s>) and $s/text() is $x and
  $s/@a/.. is $s and $s/@a << $x));

insert node attribute b { "2" } into $s;
$out := ($out, local:check("insert second attribute",
  deep-equal($s, <s a="1" b="2">x</s>) and $s/text() is $x and
  data($s/@b) instance of xs:untypedAtomic));

insert node <c/> as first into $s;
insert node <d/> as last into $s;
$out := ($out, local:check("insert first and last child",
  deep-equal($s, <s a="1" b="2"><c/>x<d/></s>) and
  $s/@b << $s/c and $s/c << $x and $x << $s/d and $s/node()[2] is $x));

delete nodes ($s/c, $s/d, $s/@b);
$out := ($out, local:check("delete back to two nodes",
  deep-equal($s, <s a="1">x</s>) and $s/text() is $x));

delete node $s/@a;
$out := ($out, local:check("delete back to one node",
  deep-equal($s, <s>x</s>) and $s/node() is $x and empty($s/@*)));

insert node <f/> into $e;
insert node "z" after $e/f;
insert node <g/> before $e/f;
$out := ($out, local:check("insert into empty element",
  deep-equal($e, <e><g/><f/>z</e>) and $e/g << $e/f and
  $e/f << $e/text() and $e/f/.. is $e));

replace node $t/text() with <w/>;
$out := ($out, local:check("replace child",
  deep-equal($t, <t k="v"><w/></t>) and $t/w/.. is $t));

replace value of node $t/@k with "v2";
$out := ($out, local:check("replace attribute value",
  string($t/@k) eq "v2" and data($t/@k) instance of xs:untypedAtomic));

rename node $t as "tt";
rename node $t/@k as "kk";
$out := ($out, local:check("rename element and attribute",
  node-name($t) eq xs:QName("tt") and string($t/@kk) eq "v2" and
  $r/tt is $t));

replace value of node $s with "new";
$out := ($out, local:check("replace element content",
  string($s) eq "new" and count($s/node()) eq 1 and
  data($s) instance of xs:untypedAtomic));

try
{
  (insert node <h/> into $e,
   insert node attribute q { "1" } into $e,
   insert node attribute q { "2" } into $e);
}
catch *
{
  $out := ($out, local:check("conflicting update raises an error", true()));
}
$out := ($out, local:check("conflicting update leaves the element unchanged",
  deep-equal($e, <e><g/><f/>z</e>) and empty($e/@*)));

$out := ($out, local:check("whole document",
  deep-equal($r, <r><s>new</s><tt kk="v2"><w/></tt><e><g/><f/>z</e></r>) and
  $s << $t and $t << $e and $doc/r is $r));

string-join($out, "&#10;")
