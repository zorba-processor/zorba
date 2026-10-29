(: ensure that all entries of the at-list are checked :)
import schema namespace s = "http://test.example.com/schema.xsd" at
 "DOESNOTEXIST.xsd", "ALSODOESNOTEXIST.xsd", "schema.xsd";

s:ContentID("bar")
