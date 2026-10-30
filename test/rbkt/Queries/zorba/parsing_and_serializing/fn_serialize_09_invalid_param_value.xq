(: testing for error SEPM0017 if the parameters are not valid against the
   schema for serialization parameters :)

let $params :=
  <serialization-parameters xmlns="http://www.w3.org/2010/xslt-xquery-serialization">
    <indent value="maybe"/>
  </serialization-parameters>
return
  fn:serialize(<a/>, $params)
