(: testing for error SEPM0019 if a parameter is specified more than once :)

let $params :=
  <serialization-parameters xmlns="http://www.w3.org/2010/xslt-xquery-serialization">
    <indent value="yes"/>
    <indent value="no"/>
  </serialization-parameters>
return
  fn:serialize(<a/>, $params)
