(: Test that importing a non-existent explicit version gets an error. :)
import module namespace vm = "http://zorba-tests.example.org/modules/ver#2.2!";

vm:version()
