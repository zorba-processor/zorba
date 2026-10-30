(: Nodes have identity and structure; atomization extracts their typed value. :)
let $p := <price currency="USD">12.50</price>
return (
  $p instance of element(),
  data($p) instance of xs:untypedAtomic,
  data($p) * 2,
  string($p/@currency),
  <price>12.50</price> is $p,
  deep-equal(<price currency="USD">12.50</price>, $p)
)
