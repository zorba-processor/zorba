import module namespace s = "http://zorba.io/modules/string";

(: The empty sequence as $input behaves as the empty string (it used to
   dereference a null item, found by the jsoniq_compile fuzzer). :)
empty(jn:members(s:analyze-string((), "b"))) and
empty(jn:members(s:analyze-string((), "b", "i")))
