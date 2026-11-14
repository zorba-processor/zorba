import schema namespace t = "http://zorba.io/test/val-lax-mixed-text"
  at "val-lax-mixed-text.xsd";

(: The simple xsi:type of k does not apply to the text of its mixed parent :)
concat("[", string(validate lax {
  <t:m>x<t:k xmlns:xs="http://www.w3.org/2001/XMLSchema"
             xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
             xsi:type="xs:integer"/> a  b </t:m>
}), "]")
