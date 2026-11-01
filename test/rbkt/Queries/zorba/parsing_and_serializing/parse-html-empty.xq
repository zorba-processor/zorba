(: x:parse-html of the empty sequence is the empty sequence; of the empty
   string, a document node without children. :)
import module namespace x = "http://zorba.io/modules/xml";

<result>
  <empty-sequence>{ empty(x:parse-html(())) }</empty-sequence>
  <document>{ x:parse-html("") instance of document-node() }</document>
  <children>{ count(x:parse-html("")/node()) }</children>
</result>
