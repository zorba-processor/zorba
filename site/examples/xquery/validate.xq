import schema namespace o = "http://example.org/order" at "lib/order.xsd";
declare namespace err = "http://www.w3.org/2005/xqt-errors";

let $order := validate { <o:order><o:qty>3</o:qty><o:price>2.50</o:price></o:order> }
return (
  <total>{ $order/o:qty * $order/o:price }</total>,
  <typed>{ data($order/o:qty) instance of xs:positiveInteger }</typed>,
  <invalid>{
    try { validate { <o:order><o:qty>-1</o:qty><o:price>2.50</o:price></o:order> } }
    catch err:XQDY0027 { "rejected" }
  }</invalid>
)
