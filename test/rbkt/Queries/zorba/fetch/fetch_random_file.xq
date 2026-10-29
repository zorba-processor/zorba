(: Fetch a random file from the filesystem :)
import module namespace fetch = "http://zorba.io/modules/fetch";

fetch:content("http://zorba-tests.example.org/random-file", "SOME_CONTENT")
