(: testing for error XPTY0004 if the wrong element name was provided :)

let $params :=
  <serialization-parameters xmlns="http://www.w3.org/2010/xslt-xquery-serialization">
    <omit-xml-declaration value="no"/>
  </serialization-parameters>
return
  fn:serialize(<a/>, $params/*:omit-xml-declaration)
