import module namespace file = "http://expath.org/ns/file";
declare namespace output = "http://www.w3.org/2010/xslt-xquery-serialization";
declare variable $rbktPath as xs:string external;
declare variable $d := concat($rbktPath, "/Queries/zorba/bom/data/");
(: The text output method writes the string, without the mark of the file. :)
string-to-codepoints(
  serialize(file:read-text(concat($d, "bom.txt")),
    <output:serialization-parameters>
      <output:method value="text"/>
    </output:serialization-parameters>))[1]
