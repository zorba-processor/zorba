(: A duplicate key is an error also when the key was seen in an earlier
   object of the same parse. :)
import module namespace jn = "http://jsoniq.org/functions";

jn:parse-json('[ { "a" : 1 }, { "b" : 1, "a" : 2, "a" : 3 } ]')
