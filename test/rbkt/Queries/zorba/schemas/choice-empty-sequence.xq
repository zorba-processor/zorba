(: XERCESC-2243: an xs:choice with an empty xs:sequence branch accepts empty
   content and each of the other branches, like (a|b)?. :)
import schema default element namespace "urn:zorba:test:choice-empty-sequence" at "choice-empty-sequence.xsd";

string-join(
  for $r in (
    validate strict { <root/> },
    validate strict { <root><a>x</a></root> },
    validate strict { <root><b>y</b></root> }
  )
  return string-join((local-name($r), string(count($r/*)), $r/*/local-name()), ":"),
  "|")
