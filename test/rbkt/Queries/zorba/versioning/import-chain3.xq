(: Test that importing different versions from inside imported modules :)
(: raises an error when the versions conflict. :)
(: (At some point in the future we may actually support this case) :)
import module namespace modA = "http://zorba-tests.example.org/modules/A";
import module namespace modB = "http://zorba-tests.example.org/modules/B";

modA:hello(),modB:hello()
