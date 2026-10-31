(: testing that parameters in other namespaces are ignored :)

let $params :=
  <serialization-parameters xmlns="http://www.w3.org/2010/xslt-xquery-serialization">
    <indent value="no"/>
    <vendor:xindent value="yes" xmlns:vendor="http://vendor.example.com/"/>
  </serialization-parameters>
return
  fn:serialize(<a/>, $params)
