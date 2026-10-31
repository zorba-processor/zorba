(: testing the item-separator parameter :)

let $params :=
  <serialization-parameters xmlns="http://www.w3.org/2010/xslt-xquery-serialization">
    <item-separator value="|"/>
  </serialization-parameters>
return
  fn:serialize(1 to 3, $params)
