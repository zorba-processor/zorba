(: Casting an empty hexBinary to base64Binary (and back) and comparing empty
   binaries used to index an empty buffer (QT3 xs-base64Binary/base64-001). :)
(
  string(xs:base64Binary(xs:hexBinary(""))) eq "",
  string(xs:hexBinary(xs:base64Binary(""))) eq "",
  xs:hexBinary("") eq xs:hexBinary(""),
  xs:base64Binary("") eq xs:base64Binary(""),
  xs:base64Binary(xs:hexBinary("")) eq xs:base64Binary("")
)
