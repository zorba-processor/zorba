(: Test that importing a version in the main module and a library module :)
(: is OK, so long as the versions don't conflict. :)
import module namespace vm = "http://zorba-tests.example.org/modules/ver#2.3";
import module namespace modB = "http://zorba-tests.example.org/modules/B";

modB:hello()
