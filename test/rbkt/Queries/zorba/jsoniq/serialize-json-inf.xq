(: The sign of an infinity survives JSON serialization, which writes it as a string. :)
declare namespace output = "http://www.w3.org/2010/xslt-xquery-serialization";

replace(
  serialize(jn:parse-json('[-1e999, 1e999]'),
    <output:serialization-parameters>
      <output:method value="json"/>
    </output:serialization-parameters>),
  "\s", "")
