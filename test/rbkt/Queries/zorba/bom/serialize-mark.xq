(: byte-order-mark="yes" writes the three bytes of the mark and nothing else. :)
declare namespace output = "http://www.w3.org/2010/xslt-xquery-serialization";

string-to-codepoints(
  serialize(<a/>,
    <output:serialization-parameters>
      <output:omit-xml-declaration value="yes"/>
      <output:byte-order-mark value="yes"/>
    </output:serialization-parameters>))[position() le 2]
