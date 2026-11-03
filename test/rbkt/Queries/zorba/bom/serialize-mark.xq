(: F&O fn:serialize: "The final stage of serialization, that is, encoding, is
 : skipped." The mark belongs to that stage: the string has none. :)
declare namespace output = "http://www.w3.org/2010/xslt-xquery-serialization";

string-to-codepoints(
  serialize(<a/>,
    <output:serialization-parameters>
      <output:omit-xml-declaration value="yes"/>
      <output:byte-order-mark value="yes"/>
    </output:serialization-parameters>))[1]
